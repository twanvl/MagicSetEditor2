//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/keyword.hpp>
#include <util/tagged_string.hpp>

class KeywordTrie;
DECLARE_TYPEOF(map<Char COMMA KeywordTrie*>);
DECLARE_TYPEOF_COLLECTION(KeywordTrie*);
DECLARE_TYPEOF_COLLECTION(KeywordP);
DECLARE_TYPEOF_COLLECTION(KeywordModeP);
DECLARE_TYPEOF_COLLECTION(KeywordParamP);
DECLARE_TYPEOF_COLLECTION(const Keyword*);

// ----------------------------------------------------------------------------- : Reflection

KeywordParam::KeywordParam()
	: optional(true)
{}

IMPLEMENT_REFLECTION(ParamReferenceType) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(script);
}

IMPLEMENT_REFLECTION(KeywordParam) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(placeholder);
	REFLECT(optional);
	REFLECT(match);
	REFLECT(script);
	REFLECT(example);
	REFLECT(refer_scripts);
}
IMPLEMENT_REFLECTION(KeywordMode) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(is_default);
}

// backwards compatability
template <typename T> void read_compat(T&, const Keyword*) {}
void read_compat(Reader& tag, Keyword* k) {
	if (!k->match.empty()) return;
	if (tag.file_app_version >= 301) return; // only for versions < 0.3.1
	String separator, parameter;
	REFLECT(separator);
	REFLECT(parameter);
	// create a match string from the keyword
	k->match = k->keyword;
	size_t start = separator.find_first_of('[');
	size_t end   = separator.find_first_of(']');
	if (start != String::npos && end != String::npos) {
		k->match += separator.substr(start + 1, end - start - 1);
	}
	if (parameter == _("no parameter")) {
		parameter.clear(); // was used for magic to indicate absence of parameter
	}
	if (!parameter.empty()) {
		k->match += _("<atom-param>") + parameter + _("</atom-param>");
	}
}

IMPLEMENT_REFLECTION(Keyword) {
	REFLECT(keyword);
	read_compat(tag, this);
	REFLECT(match);
	REFLECT(reminder);
	REFLECT(rules);
	REFLECT(mode);
}


size_t Keyword::findMode(const vector<KeywordModeP>& modes) const {
	// find
	size_t id = 0;
	FOR_EACH_CONST(m, modes) {
		if (mode == m->name) return id;
		++id;
	}
	// default
	id = 0;
	FOR_EACH_CONST(m, modes) {
		if (m->is_default) return id;
		++id;
	}
	// not found
	return 0;
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

void Keyword::prepare(const vector<KeywordParamP>& param_types, bool force) {
	if (!force && matchRe.IsValid()) return;
	parameters.clear();
	// Prepare regex
	String regex = _("(");
	vector<KeywordParamP>::const_iterator param = parameters.begin();
	// Parse the 'match' string
	for (size_t i = 0 ; i < match.size() ;) {
		Char c = match.GetChar(i);
		if (is_substr(match, i, _("<atom-param"))) {
			// parameter, determine type...
			size_t start = skip_tag(match, i), end = match_close_tag(match, i);
			String type = match.substr(start, end-start);
			// find parameter type 'type'
			KeywordParamP p;
			FOR_EACH_CONST(pt, param_types) {
				if (pt->name == type) {
					p = pt;
					break;
				}
			}
			if (!p) {
				throw InternalError(_("Unknown keyword parameter type: ") + type);
			}
			parameters.push_back(p);
			// modify regex
			regex += _(")(") + make_non_capturing(p->match) + _(")") + (p->optional ? _("?") : _("")) + _("(");
			i = skip_tag(match, end);
		} else {
			regex += regex_escape(c);
			i++;
		}
	}
	regex += _(")");
	if (matchRe.Compile(regex, wxRE_ADVANCED)) {
		// not valid if it matches "", that would make MSE hang
		valid = !matchRe.Matches(_(""));
	} else {
		valid = false;
		throw InternalError(_("Error creating match regex"));
	}
}

// ----------------------------------------------------------------------------- : KeywordTrie

/// A node in a trie to match keywords
class KeywordTrie {
  public:
	KeywordTrie();
	~KeywordTrie();
	
	map<Char, KeywordTrie*> children;    ///< children after a given character (owned)
	KeywordTrie*            on_any_star; ///< children on /.*/ (owned or this)
	vector<const Keyword*>  finished;    ///< keywordss that end in this node
	
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
	if (kw.match.empty() || !kw.valid) return; // can't handle empty keywords
	if (!root) {
		root = new KeywordTrie;
		root->on_any_star = root;
	}
	KeywordTrie* cur = root->insertAnyStar();
	for (size_t i = 0 ; i < kw.match.size() ;) {
		Char c = kw.match.GetChar(i);
		if (is_substr(kw.match, i, _("<atom-param"))) {
			// tag, parameter, match anything
			cur = cur->insertAnyStar();
			i = match_close_tag_end(kw.match, i);
		} else {
			cur = cur->insert(c);
			i++;
		}
	}
	// now cur is the trie after matching the keyword anywhere in the input text
	cur->finished.push_back(&kw);
}

void KeywordDatabase::prepare_parameters(const vector<KeywordParamP>& ps, const vector<KeywordP>& kws) {
	FOR_EACH_CONST(kw, kws) {
		kw->prepare(ps);
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

#ifdef _DEBUG
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
#endif

String KeywordDatabase::expand(const String& text,
                               const ScriptValueP& expand_default,
                               const ScriptValueP& combine_script,
                               Context& ctx) const {
	assert(combine_script);
	
	// Remove all old reminder texts
	String s = remove_tag_contents(text, _("<atom-reminder>"));
	s = remove_tag_contents(s, _("<atom-keyword>")); // OLD, TODO: REMOVEME
	s = remove_tag_contents(s, _("<atom-kwpph>"));
	s = remove_tag(s, _("<keyword-param"));
	s = remove_tag(s, _("<param-"));
	String untagged = untag_no_escape(s);
	
	if (!root) return s;
	
	String result;
	
	// Find keywords
	while (!s.empty()) {
		vector<KeywordTrie*> current; // current location(s) in the trie
		vector<KeywordTrie*> next;    // location(s) after this step
		set<const Keyword*>  used;    // keywords already investigated
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
				map<Char,KeywordTrie*>::const_iterator it = kt->children.find(c);
				if (it != kt->children.end()) {
					next.push_back(it->second);
				}
				// TODO: on any star first or last?
				if (kt->on_any_star) {
					next.push_back(kt->on_any_star);
				}
			}
			// next becomes current
			swap(current, next);
			next.clear();
			closure(current);
			// are we done?
			FOR_EACH(n, current) {
				FOR_EACH(f, n->finished) {
					const Keyword* kw = f;
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
							
							// Split the keyword, set parameters in context
							String total; // the total keyword
							size_t match_count = kw->matchRe.GetMatchCount();
							assert(match_count - 1 == 1 + 2 * kw->parameters.size());
							for (size_t j = 1 ; j < match_count ; ++j) {
								// j = odd -> text
								// j = even -> parameter #(j/2)
								size_t start_u, len_u;
								kw->matchRe.GetMatch(&start_u, &len_u, j);
								// note: start_u can be (uint)-1 when len_u == 0
								size_t part_end = len_u > 0 ? untagged_to_index(s, start_u + len_u, true) : start;
								String part = s.substr(start, part_end - start);
								if ((j % 2) == 0) {
									// parameter
									KeywordParam& kwp = *kw->parameters[j/2-1];
									String param = untagged.substr(start_u, len_u); // untagged version
									if (param.empty()) {
										// placeholder
										param = _("<atom-kwpph>") + (kwp.placeholder.empty() ? kwp.name : kwp.placeholder) + _("</atom-kwpph>");
										part  = part + param; // keep tags
									} else if (kw->parameters[j/2-1]->script) {
										// apply parameter script
										ctx.setVariable(_("input"), to_script(part));
										part  = kwp.script.invoke(ctx)->toString();
										ctx.setVariable(_("input"), to_script(part));
										param = kwp.script.invoke(ctx)->toString();
									}
									String param_type = replace_all(replace_all(replace_all(kwp.name,
															_("("),_("-")),
															_(")"),_("-")),
															_(" "),_("-"));
									part  = _("<param-") + param_type + _(">") + part  + _("</param-") + param_type + _(">");
									param = _("<param-") + param_type + _(">") + param + _("</param-") + param_type + _(">");
									ctx.setVariable(String(_("param")) << (int)(j/2), to_script(param));
								}
								total += part;
								start = part_end;
							}
							ctx.setVariable(_("mode"), to_script(kw->mode));
							
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
								ctx.setVariable(_("keyword"),  to_script(total));
								ctx.setVariable(_("reminder"), to_script(reminder));
								result +=  _("<kw-"); result += expand_type; result += _(">");
								result += combine_script->eval(ctx)->toString();
								result += _("</kw-"); result += expand_type; result += _(">");
							} else {
								result +=  _("<kw-"); result += expand_type; result += _(">");
								result += total;
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
