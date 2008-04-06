//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/keyword.hpp>
#include <util/tagged_string.hpp>

class KeywordTrie;
DECLARE_TYPEOF(map<Char COMMA KeywordTrie*>);
DECLARE_TYPEOF_COLLECTION(KeywordTrie*);
DECLARE_TYPEOF_COLLECTION(KeywordP);
DECLARE_TYPEOF_COLLECTION(KeywordModeP);
DECLARE_TYPEOF_COLLECTION(KeywordParamP);
DECLARE_TYPEOF_COLLECTION(const Keyword*);
DECLARE_POINTER_TYPE(KeywordParamValue);
class Value;
DECLARE_DYNAMIC_ARG(Value*, value_being_updated);

// ----------------------------------------------------------------------------- : Reflection

KeywordParam::KeywordParam()
	: optional(true)
	, eat_separator(true)
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
	REFLECT(separator_before_is);
	REFLECT(separator_after_is);
	REFLECT(eat_separator);
	REFLECT(script);
	REFLECT(reminder_script);
	REFLECT(separator_script);
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

/*//%%
String KeywordParam::make_separator_before() const {
	// decode regex; find a string that matches it
	String ret;
	int disabled = 0;
	for (size_t i = 0 ; i < separator_before_is.size() ; ++i) {
		Char c = separator_before_is.GetChar(i);
		if (c == _('(')) {
			if (disabled) ++disabled;
		} else if (c == _(')')) {
			if (disabled) --disabled;
		} else if (!disabled) {
			if (c == _('|')) {
				disabled = 1; // disable after |
			} else if (c == _('+') || c == _('*') || c == _('?') || c == _('^') || c == _('$')) {
				// ignore
			} else if (c == _('\\') && i + 1 < separator_before_is.size()) {
				// escape
				ret += separator_before_is.GetChar(++i);
			} else if (c == _('[') && i + 1 < separator_before_is.size()) {
				// character class
				c = separator_before_is.GetChar(++i);
				if (c != _('^')) ret += c;
				// ignore the rest of the class
				for ( ++i ; i < separator_before_is.size() ; ++i) {
					c = separator_before_is.GetChar(i);
					if (c == _(']')) break;
				}
			} else {
				ret += c;
			}
		}
	}
	return ret;
}*/
void KeywordParam::compile() {
	// compile separator_before
	if (!separator_before_is.empty() && !separator_before_re.IsValid()) {
		separator_before_re.Compile(_("^") + separator_before_is, wxRE_ADVANCED);
		if (eat_separator) {
			separator_before_eat.Compile(separator_before_is + _("$"), wxRE_ADVANCED);
		}
	}
	// compile separator_after
	if (!separator_after_is.empty() && !separator_after_re.IsValid()) {
		separator_after_re.Compile(separator_after_is + _("$"), wxRE_ADVANCED);
		if (eat_separator) {
			separator_after_eat.Compile(_("^") + separator_after_is, wxRE_ADVANCED);
		}
	}
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

void Keyword::prepare(const vector<KeywordParamP>& param_types, bool force) {
	if (!force && match_re.IsValid()) return;
	parameters.clear();
	// Prepare regex
	String regex;
	String text; // normal, non-regex, text
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
				// throwing an error can mean a set will not be loaded!
				// instead, simply disable the keyword
				//throw InternalError(_("Unknown keyword parameter type: ") + type);
				handle_error(_("Unknown keyword parameter type: ") + type, true, false);
				valid = false;
				return;
			}
			parameters.push_back(p);
			// modify regex : match text before
			p->compile();
			if (p->separator_before_eat.IsValid() && p->separator_before_eat.Matches(text)) {
				// remove the separator from the text to prevent duplicates
				size_t start, len;
				p->separator_before_eat.GetMatch(&start, &len);
				text = text.substr(0, start);
			}
			regex += _("(") + regex_escape(text) + _(")");
			text.clear();
			// modify regex : match parameter
			regex += _("(") + make_non_capturing(p->match) + (p->optional ? _(")?") : _(")"));
			i = skip_tag(match, end);
			// eat separator_after?
			if (p->separator_after_eat.IsValid() && p->separator_after_eat.Matches(match.substr(i))) {
				size_t start, len;
				p->separator_before_eat.GetMatch(&start, &len);
				i += start + len;
			}
		} else {
			text += c;
			i++;
		}
	}
	regex += _("(") + regex_escape(text) + _(")");
	regex = _("\\y") + regex + _("(?=$|[^a-zA-Z0-9])"); // only match whole words
	if (match_re.Compile(regex, wxRE_ADVANCED)) {
		// not valid if it matches "", that would make MSE hang
		valid = !match_re.Matches(_(""));
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

IMPLEMENT_DYNAMIC_ARG(KeywordUsageStatistics*, keyword_usage_statistics, nullptr);

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
	// Create root
	if (!root) {
		root = new KeywordTrie;
		root->on_any_star = root;
	}
	KeywordTrie* cur = root->insertAnyStar();
	// Add to trie
	String text; // normal text
	size_t param = 0;
	bool only_star = true;
	for (size_t i = 0 ; i < kw.match.size() ;) {
		Char c = kw.match.GetChar(i);
		if (is_substr(kw.match, i, _("<atom-param"))) {
			i = match_close_tag_end(kw.match, i);
			// parameter, is there a separator we should eat?
			if (param < kw.parameters.size()) {
				wxRegEx& sep_before = kw.parameters[param]->separator_before_eat;
				wxRegEx& sep_after  = kw.parameters[param]->separator_after_eat;
				if (sep_before.IsValid() && sep_before.Matches(text)) {
					// remove the separator from the text to prevent duplicates
					size_t start, len;
					sep_before.GetMatch(&start, &len);
					text = text.substr(0, start);
				}
				if (sep_after.IsValid() && sep_after.Matches(kw.match.substr(i))) {
					size_t start, len;
					sep_after.GetMatch(&start, &len);
					i += start + len;
				}
			}
			++param;
			// match anything
			cur = cur->insert(text);
			text.clear();
			cur = cur->insertAnyStar();
			// enough?
			if (!only_star) {
				// If we have matched anything specific, this is a good time to stop
				// it doesn't really matter how long we go on, since the trie is only used
				// as an optimization to not have to match lots of regexes.
				// As an added bonus, we get a better behaviour of matching earlier keywords first.
				break;
			}
		} else {
			text += c;
			i++;
			only_star = false;
		}
	}
	cur = cur->insert(text);
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
	
	// Clean up usage statistics
	KeywordUsageStatistics* stat = keyword_usage_statistics();
	Value* stat_key = value_being_updated();
	if (stat && stat_key) {
		for (size_t i = stat->size() - 1 ; i + 1 > 0 ; --i) { // loop backwards
			if ((*stat)[i].first == stat_key) {
				stat->erase(stat->begin() + i);
			}
		}
	}
	
	// Remove all old reminder texts
	String s = remove_tag_contents(text, _("<atom-reminder"));
	s = remove_tag_contents(s, _("<atom-keyword")); // OLD, TODO: REMOVEME
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
			// in the MSVC stl clear frees memory, that is a waste, because we need it again in the next iteration
			//next.clear();
			next.resize(0);
			closure(current);
			// are we done?
			FOR_EACH(n, current) {
				FOR_EACH(f, n->finished) {
					const Keyword* kw = f;
					if (used.insert(kw).second) {
						// we have found a possible match, which we have not seen before
						assert(kw->match_re.IsValid());
						
						// try to match it against the *untagged* string
						if (kw->match_re.Matches(untagged)) {
							// Everything before the keyword
							size_t start_u, len_u;
							kw->match_re.GetMatch(&start_u, &len_u, 0);
							size_t start = untagged_to_index(s, start_u, true),
							       end   = untagged_to_index(s, start_u + len_u, true);
							if (start == end) continue; // don't match empty keywords
							// copy text before keyword
							result += remove_tag(s.substr(0, start), _("<kw-"));
							
							// a part of s has not been searched for <kw- tags
							// this can happen when the trie incorrectly matches too early
							for (size_t j = i+1 ; j < start ;) {
								Char c = s.GetChar(j);
								if (c == _('<')) {
									if (is_substr(s, j, _("<kw-")) && j + 4 < s.size()) {
										expand_type = s.GetChar(j + 4); // <kw-?>
									} else if (is_substr(s, j, _("</kw-"))) {
										expand_type = 'a';
									}
									j = skip_tag(s, j);
								} else {
									++j;
								}
							}
							
							// Split the keyword, set parameters in context
							String total; // the total keyword
							size_t match_count = kw->match_re.GetMatchCount();
							assert(match_count - 1 == 1 + 2 * kw->parameters.size());
							for (size_t j = 1 ; j < match_count ; ++j) {
								// we start counting at 1, so
								// j = 1 mod 2 -> text
								// j = 0 mod 2 -> parameter  #((j-1)/2) == (j/2-1)
								size_t start_u, len_u;
								kw->match_re.GetMatch(&start_u, &len_u, j);
								// note: start_u can be (uint)-1 when len_u == 0
								size_t part_end = len_u > 0 ? untagged_to_index(s, start_u + len_u, true) : start;
								String part = s.substr(start, part_end - start);
								// strip left over </kw tags
								part = remove_tag(part,_("</kw-"));
								if ((j % 2) == 0) {
									// parameter
									KeywordParam& kwp = *kw->parameters[j/2-1];
									String param = untagged.substr(start_u, len_u); // untagged version
									// strip separator_before
									String separator_before, separator_after;
									if (kwp.separator_before_re.IsValid() && kwp.separator_before_re.Matches(param)) {
										size_t s_start, s_len; // start should be 0
										kwp.separator_before_re.GetMatch(&s_start, &s_len);
										separator_before = param.substr(0, s_start + s_len);
										param = param.substr(s_start + s_len);
										// strip from tagged version
										size_t end_t = untagged_to_index(part, s_start + s_len, false);
										part = get_tags(part, 0, end_t, true, true) + part.substr(end_t);
										// transform?
										if (kwp.separator_script) {
											ctx.setVariable(_("input"), to_script(separator_before));
											separator_before = kwp.separator_script.invoke(ctx)->toString();
										}
									}
									// strip separator_after
									if (kwp.separator_after_re.IsValid() && kwp.separator_after_re.Matches(param)) {
										size_t s_start, s_len; // start + len should be param.size()
										kwp.separator_after_re.GetMatch(&s_start, &s_len);
										separator_after = param.substr(s_start);
										param = param.substr(0, s_start);
										// strip from tagged version
										size_t start_t = untagged_to_index(part, s_start, false);
										part = part.substr(0, start_t) + get_tags(part, start_t, part.size(), true, true);
										// transform?
										if (kwp.separator_script) {
											ctx.setVariable(_("input"), to_script(separator_after));
											separator_after = kwp.separator_script.invoke(ctx)->toString();
										}
									}
									// to script
									KeywordParamValueP script_param(new KeywordParamValue(kwp.name, separator_before, separator_after, param));
									KeywordParamValueP script_part (new KeywordParamValue(kwp.name, separator_before, separator_after, part));
									// process param
									if (param.empty()) {
										// placeholder
										script_param->value = _("<atom-kwpph>") + (kwp.placeholder.empty() ? kwp.name : kwp.placeholder) + _("</atom-kwpph>");
										script_part->value  = part + script_param->value; // keep tags
									} else {
										// apply parameter script
										if (kwp.script) {
											ctx.setVariable(_("input"), script_part);
											script_part->value  = kwp.script.invoke(ctx)->toString();
										}
										if (kwp.reminder_script) {
											ctx.setVariable(_("input"), script_param);
											script_param->value = kwp.reminder_script.invoke(ctx)->toString();
										}
									}
									part  = separator_before + script_part->toString() + separator_after;
									ctx.setVariable(String(_("param")) << (int)(j/2), script_param);
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
							
							// Add to usage statistics
							if (stat && stat_key) {
								stat->push_back(make_pair(stat_key, kw));
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

// ----------------------------------------------------------------------------- : KeywordParamValue

ScriptType KeywordParamValue::type() const { return SCRIPT_STRING; }
String KeywordParamValue::typeName() const { return _("keyword parameter"); }

KeywordParamValue::operator String() const {
	String safe_type = replace_all(replace_all(replace_all(type_name,
							_("("),_("-")),
							_(")"),_("-")),
							_(" "),_("-"));
	return _("<param-") + safe_type + _(">") + value  + _("</param-") + safe_type + _(">");
}

KeywordParamValue::operator int()    const { return *to_script(value); } // a bit of a hack
KeywordParamValue::operator double() const { return *to_script(value); }
KeywordParamValue::operator Color()  const { return *to_script(value); }
int KeywordParamValue::itemCount()   const { return  to_script(value)->itemCount(); }

ScriptValueP KeywordParamValue::getMember(const String& name) const {
	if (name == _("type"))             return to_script(type_name);
	if (name == _("separator before")) return to_script(separator_before);
	if (name == _("separator after"))  return to_script(separator_after);
	if (name == _("value"))            return to_script(value);
	if (name == _("param"))            return to_script(value);
	return ScriptValue::getMember(name);
}
