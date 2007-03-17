//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/keyword.hpp>
#include <util/tagged_string.hpp>

class KeywordTrie;
DECLARE_TYPEOF(map<Char COMMA KeywordTrie*>);
DECLARE_TYPEOF_COLLECTION(KeywordTrie*);
DECLARE_TYPEOF_COLLECTION(KeywordP);
DECLARE_TYPEOF_COLLECTION(KeywordParamP);
DECLARE_TYPEOF_COLLECTION(KeywordExpansionP);
DECLARE_TYPEOF_COLLECTION(const KeywordExpansion*);

// ----------------------------------------------------------------------------- : Reflection

IMPLEMENT_REFLECTION(KeywordParam) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(match);
	REFLECT(script);
}
IMPLEMENT_REFLECTION(KeywordMode) {
	REFLECT(name);
	REFLECT(description);
}
IMPLEMENT_REFLECTION(KeywordExpansion) {
	REFLECT(match);
	REFLECT(reminder);
}

// backwards compatability
template <typename T> void read_compat(T&, const Keyword*) {}
void read_compat(Reader& tag, Keyword* k) {
	String separator, parameter, reminder;
	REFLECT(separator);
	REFLECT(parameter);
	REFLECT(reminder);
	if (!separator.empty() || !parameter.empty() || !reminder.empty()) {
		// old style keyword declaration, no separate expansion
		KeywordExpansionP e(new KeywordExpansion);
		e->match = k->keyword;
		size_t start = separator.find_first_of('[');
		size_t end   = separator.find_first_of(']');
		if (start != String::npos && end != String::npos) {
			e->match += separator.substr(start + 1, end - start - 1);
		}
		if (!parameter.empty()) {
			e->match += _("<param>") + parameter + _("</param>");
		}
		e->reminder.set(reminder);
		k->expansions.push_back(e);
	}
}

IMPLEMENT_REFLECTION(Keyword) {
	REFLECT(keyword);
	read_compat(tag, this);
	REFLECT(expansions);
	REFLECT(rules);
//	REFLECT(mode);
}


// ----------------------------------------------------------------------------- : Regex stuff

/// Make sure the given regex does no capturing
/** Basicly replaces "(" with "(?:" */
String make_non_capturing(const String& re) {
	String ret;
	bool escape = false, bracket = false, capture = false;
	FOR_EACH_CONST(c, re) {
		if (capture && c != _('?')) {
			// change this capture into a non-capturing "(" by appending "?:"
			ret += _("?:");
			capture = false;
		}
		if (escape) { // second char of escape sequence
			escape = false;
		} else if (c == _('\\')) { // start of escape sequence
			escape = true;
		} else if (c == _('[')) { // start of [...]
			bracket = true;
		} else if (c == _(']')) { // end of [...]
			bracket = false;
		} else if (bracket && c == _('(')) {
			// wx has a bug, it counts the '(' in "[(]" as a matching group
			// escape it so wx doesn't see it
			ret += _('\\');
		} else if (c == _('(')) { // start of capture?
			capture = true;
		}
		ret += c;
	}
	return ret;
}

/// Escape a single character for use in regular expressions
String regex_escape(Char c) {
	if (c == _('(') || c == _(')') || c == _('[') || c == _(']') || c == _('{') ||
	    c == _('.') || c == _('^') || c == _('$') || c == _('#') || c == _('\\') ||
	    c == _('|') || c == _('+') || c == _('*') || c == _('?')) {
		// c needs to be escaped
		return _("\\") + String(1,c);
	} else {
		return String(1,c);
	}
}

void KeywordExpansion::prepare(const vector<KeywordParamP>& param_types, bool force) {
	if (!force && matchRe.IsValid()) return;
	parameters.clear();
	// Prepare regex
//	String regex;
	vector<KeywordParamP>::const_iterator param = parameters.begin();
	// Parse the 'match' string
	for (size_t i = 0 ; i < match.size() ;) {
		Char c = match.GetChar(i);
		if (is_substr(match, i, _("<param"))) {
			// parameter, determine type...
			size_t start = skip_tag(match, i), end = match_close_tag(match, i);
			String type = match.substr(start, end-start);
			// find parameter type 'type'
			KeywordParam* p = nullptr;
			FOR_EACH_CONST(pt, param_types) {
				if (pt->name == type) {
					p = pt.get();
					break;
				}
			}
			if (!p) {
				throw InternalError(_("Unknown keyword parameter type: ") + type);
			}
			// modify regex
			regex += _("(") + make_non_capturing(p->match) + _(")");
			i = skip_tag(match, end);
		} else {
			regex += regex_escape(c);
			i++;
		}
	}
	if (!matchRe.Compile(regex, wxRE_ADVANCED)) {
		throw InternalError(_("Error creating match regex"));
	}
}

// ----------------------------------------------------------------------------- : KeywordTrie

/// A node in a trie to match keywords
class KeywordTrie {
  public:
	KeywordTrie();
	~KeywordTrie();
	
	map<Char, KeywordTrie*>   children;    ///< children after a given character (owned)
	KeywordTrie*              on_any_star; ///< children on /.*/ (owned or this)
	vector<const KeywordExpansion*> finished;    ///< keywords expansions that end in this node
	
	/// Insert nodes representing the given character
	/** return the node where the evaluation will be after matching the character */
	KeywordTrie* insert(Char match);
	/// Insert nodes representing the given string
	/** return the node where the evaluation will be after matching the string */
	KeywordTrie* insert(const String& match);

	/// Insert nodes representing the regex /.*/
	/** return the node where the evaluation will be after matching that regex */
	KeywordTrie* insertAnyStar();
};


KeywordTrie::KeywordTrie()
	: on_any_star(nullptr)
	, finished(nullptr)
{}

KeywordTrie::~KeywordTrie() {
	FOR_EACH(c, children) {
		delete c.second;
	}
	if (on_any_star != this) delete on_any_star;
}

KeywordTrie* KeywordTrie::insert(Char c) {
	KeywordTrie*& child = children[c];
	if (!child) child = new KeywordTrie;
	return child;
}
KeywordTrie* KeywordTrie::insert(const String& match) {
	KeywordTrie* cur = this;
	FOR_EACH_CONST(c, match) {
		cur = cur->insert(c);
	}
	return cur;
}

KeywordTrie* KeywordTrie::insertAnyStar() {
	if (!on_any_star) on_any_star = new KeywordTrie;
	on_any_star->on_any_star = on_any_star; // circular reference to itself
	return on_any_star;
}


// ----------------------------------------------------------------------------- : KeywordDatabase

KeywordDatabase::KeywordDatabase()
	: root(nullptr)
{}

KeywordDatabase::~KeywordDatabase() {
	clear();
}

void KeywordDatabase::clear() {
	delete root;
	root = nullptr;
}

void KeywordDatabase::add(const vector<KeywordP>& kws) {
	FOR_EACH_CONST(kw, kws) {
		add(*kw);
	}
}
void KeywordDatabase::add(const Keyword& kw) {
	FOR_EACH_CONST(e, kw.expansions) {
		add(*e);
	}
}

void KeywordDatabase::add(const KeywordExpansion& e) {
	if (!root) {
		root = new KeywordTrie;
		root->on_any_star = root;
	}
	KeywordTrie* cur = root->insertAnyStar();
	for (size_t i = 0 ; i < e.match.size() ;) {
		Char c = e.match.GetChar(i);
		if (is_substr(e.match, i, _("<param"))) {
			// tag, parameter, match anything
			cur = cur->insertAnyStar();
			i = match_close_tag_end(e.match, i);
		} else {
			cur = cur->insert(c);
			i++;
		}
	}
	// now cur is the trie after matching the keyword anywhere in the input text
	cur->finished.push_back(&e);
}

void KeywordDatabase::prepare_parameters(const vector<KeywordParamP>& ps, const vector<KeywordP>& kws) {
	FOR_EACH_CONST(kw, kws) {
		prepare_parameters(ps, *kw);
	}
}
void KeywordDatabase::prepare_parameters(const vector<KeywordParamP>& ps, const Keyword& kw) {
	FOR_EACH_CONST(e, kw.expansions) {
		e->prepare(ps);
	}
}

// ----------------------------------------------------------------------------- : KeywordDatabase : matching

// transitive closure of a state, follow all on_any_star links
void closure(vector<KeywordTrie*>& state) {
	for (size_t j = 0 ; j < state.size() ; ++j) {
		if (state[j]->on_any_star && state[j]->on_any_star != state[j]) {
			state.push_back(state[j]->on_any_star);
		}
	}
}

//DEBUG
void dump(int i, KeywordTrie* t) {
	FOR_EACH(c, t->children) {
		wxLogDebug(String(i,_(' ')) + c.first + _("     ") + String::Format(_("%p"),c.second));
		dump(i+2, c.second);
	}
	if (t->on_any_star) {
		wxLogDebug(String(i,_(' ')) + _(".*") + _("     ") + String::Format(_("%p"),t->on_any_star));
		if (t->on_any_star != t) dump(i+2, t->on_any_star);
	}
}

String KeywordDatabase::expand(const String& text,
                               const ScriptValueP& expand_default,
                               const ScriptValueP& combine_script,
                               Context& ctx) const {
	// DEBUG: dump db
	//dump(0, root);
	
	assert(combine_script);
	
	// Remove all old reminder texts
	String s = remove_tag_contents(text, _("<atom-reminder>"));
	s = remove_tag_contents(s, _("<atom-keyword>")); // OLD, TODO: REMOVEME
	s = remove_tag(s, _("<keyword-param"));
	String untagged = untag_no_escape(s);
	
	if (!root) return s;
	
	String result;
	
	// Find keywords
	while (!s.empty()) {
		vector<KeywordTrie*> current; // current location(s) in the trie
		vector<KeywordTrie*> next;    // location(s) after this step
		set<const KeywordExpansion*> used; // keywords already investigated
		current.push_back(root);
		closure(current);
		char expand_type = 'a'; // is the keyword expanded? From <kw-?> tag
		                        // Possible values are:
		                        //  - '0' = reminder text explicitly hidden
		                        //  - '1' = reminder text explicitly shown
		                        //  - 'a' = reminder text in default state, hidden
		                        //  - 'A' = reminder text in default state, shown
		
		for (size_t i = 0 ; i < s.size() ;) {
			Char c = s.GetChar(i);
			// tag?
			if (c == _('<')) {
				if (is_substr(s, i, _("<kw-")) && i + 4 < s.size()) {
					expand_type = s.GetChar(i + 4); // <kw-?>
					s = s.erase(i, skip_tag(s,i)-i); // remove the tag from the string
				} else if (is_substr(s, i, _("</kw-"))) {
					expand_type = 'a';
					s = s.erase(i, skip_tag(s,i)-i); // remove the tag from the string
				} else {
					i = skip_tag(s, i);
				}
				continue;
			} else {
				++i;
			}
			// find 'next' trie node set matching c
			FOR_EACH(kt, current) {
				if (kt->on_any_star) {
					next.push_back(kt->on_any_star);
				}
				map<Char,KeywordTrie*>::const_iterator it = kt->children.find(c);
				if (it != kt->children.end()) {
					next.push_back(it->second);
					wxLogDebug(c + String(_(" -> ")) + String::Format(_("%p"), it->second));
				}
			}
			// next becomes current
			swap(current, next);
			next.clear();
			closure(current);
			// are we done?
			FOR_EACH(n, current) {
				FOR_EACH(f, n->finished) {
					const KeywordExpansion* kw = f;
					if (used.insert(kw).second) {
						// we have found a possible match, which we have not seen before
						assert(kw->matchRe.IsValid());
						
						// try to match it against the *untagged* string
						if (kw->matchRe.Matches(untagged)) {
							// Everything before the keyword
							size_t start_u, len_u;
							kw->matchRe.GetMatch(&start_u, &len_u, 0);
							size_t start = untagged_to_index(s, start_u, true),
							       end   = untagged_to_index(s, start_u + len_u, true);
							result += s.substr(0, start);
							
							// Set parameters in context
							size_t match_count = kw->matchRe.GetMatchCount();
							for (size_t j = 1 ; j < match_count ; ++j) {
								size_t start_u, len_u;
								kw->matchRe.GetMatch(&start_u, &len_u, j);
								String param = untagged.substr(start_u, len_u);
								if (j-1 < kw->parameters.size() && kw->parameters[j-1]->script) {
									// apply parameter script
									param = kw->parameters[j-1]->script.invoke(ctx)->toString();
								}
								ctx.setVariable(String(_("param")) << (int)j, toScript(param));
							}
							ctx.setVariable(_("mode"), toScript(kw->mode));
							
							// Show reminder text?
							bool expand = expand_type == _('1');
							if (!expand && expand_type != _('0')) {
								// default expand, determined by script
								expand = expand_default && (bool)*expand_default->eval(ctx);
								expand_type = expand ? _('A') : _('a');
							}
							
							// Combine keyword & reminder with result
							if (expand) {
								String reminder = kw->reminder.invoke(ctx)->toString();
								ctx.setVariable(_("keyword"),  toScript(s.substr(start, end - start)));
								ctx.setVariable(_("reminder"), toScript(reminder));
								result +=  _("<kw-"); result += expand_type; result += _(">");
								result += combine_script->eval(ctx)->toString();
								result += _("</kw-"); result += expand_type; result += _(">");
							} else {
								result +=  _("<kw-"); result += expand_type; result += _(">");
								result += s.substr(start, end - start);
								result += _("</kw-"); result += expand_type; result += _(">");
							}
							
							// After keyword
							s        = s.substr(end);
							untagged = untagged.substr(start_u + len_u);
							used.clear();
							expand_type = _('a');
							goto matched_keyword;
						}
						
					}
				}
			}
		}
		// Remainder of the string
		result += s; s.clear();
		
		matched_keyword:;
	}
	
	return result;
}
