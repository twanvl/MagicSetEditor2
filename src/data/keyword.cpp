//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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

#define USE_CASE_INSENSITIVE_KEYWORDS 1

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

bool Keyword::contains(String const& query) const {
	if (find_i(keyword,query) != String::npos) return true;
	if (find_i(rules,query) != String::npos) return true;
	if (find_i(match,query) != String::npos) return true;
	if (find_i(reminder.get(),query) != String::npos) return true;
	return false;
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
	if (!separator_before_is.empty() && separator_before_re.empty()) {
		separator_before_re.assign(_("^") + separator_before_is);
		if (eat_separator) {
			separator_before_eat.assign(separator_before_is + _("$"));
		}
	}
	// compile separator_after
	if (!separator_after_is.empty() && separator_after_re.empty()) {
		separator_after_re.assign(separator_after_is + _("$"));
		if (eat_separator) {
			separator_after_eat.assign(_("^") + separator_after_is);
		}
	}
}
void KeywordParam::eat_separator_before(String& text) {
	if (separator_before_eat.empty()) return;
	Regex::Results result;
	if (separator_before_eat.matches(result, text)) {
		// keep only stuff before the separator
		assert(result.position() + result.size() == text.size());
		text.resize(result.position());
	}
}
void KeywordParam::eat_separator_after(const String& text, size_t& i) {
	if (separator_after_eat.empty()) return;
	Regex::Results result;
	if (separator_after_eat.matches(result, text.begin() + i, text.end())) {
		// advance past the separator
		assert(result.position() == 0);
		i += result.length();
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
	if (!force && !match_re.empty()) return;
	parameters.clear();
	// Prepare regex
	String regex;
	String text; // normal, non-regex, text
	vector<KeywordParamP>::const_iterator param = parameters.begin();
	#if USE_CASE_INSENSITIVE_KEYWORDS
		regex = _("(?i)"); // case insensitive matching
	#endif
	// Parse the 'match' string
	for (size_t i = 0 ; i < match.size() ;) {
		Char c = match.GetChar(i);
		if (is_substr(match, i, _("<atom-param"))) {
			// parameter, determine type...
			size_t start = skip_tag(match, i), end = match_close_tag(match, i);
			String type = match.substr(start, end-start);
			// find parameter type 'type'
			KeywordParamP param;
			FOR_EACH_CONST(pt, param_types) {
				if (pt->name == type) {
					param = pt;
					break;
				}
			}
			if (!param) {
				// throwing an error can mean a set will not be loaded!
				// instead, simply disable the keyword
				//throw InternalError(_("Unknown keyword parameter type: ") + type);
				handle_error(_("Unknown keyword parameter type: ") + type);
				valid = false;
				return;
			}
			parameters.push_back(param);
			// modify regex : match text before
			param->compile();
			// remove the separator from the text to prevent duplicates
			param->eat_separator_before(text);
			regex += _("(") + regex_escape(text) + _(")");
			text.clear();
			// modify regex : match parameter
			regex += _("(") + make_non_capturing(param->match) + (param->optional ? _(")?") : _(")"));
			i = skip_tag(match, end);
			// eat separator_after?
			param->eat_separator_after(match, i);
		} else {
			text += c;
			i++;
		}
	}
	regex += _("(") + regex_escape(text) + _(")");
	#if USE_BOOST_REGEX
		regex = _("\\<")
	#else
		regex = _("\\y")
	#endif
	      + regex + _("(?=$|[^a-zA-Z0-9\\(])"); // only match whole words
	match_re.assign(regex);
	// not valid if it matches "", that would make MSE hang
	valid = !match_re.matches(_(""));
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
	#if USE_CASE_INSENSITIVE_KEYWORDS
		c = toLower(c); // case insensitive matching
	#endif
	KeywordTrie*& child = children[c];
	if (!child) child = new KeywordTrie;
	return child;
}
KeywordTrie* KeywordTrie::insert(const String& match) {
	KeywordTrie* cur = this;
	FOR_EACH_CONST(c, match) {
		cur = cur->insert(static_cast<Char>(c));
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
				kw.parameters[param]->eat_separator_before(text);
				kw.parameters[param]->eat_separator_after(kw.match, i);
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
                               const ScriptValueP& match_condition,
                               const ScriptValueP& expand_default,
                               const ScriptValueP& combine_script,
                               Context& ctx) const {
	assert(combine_script);
	assert_tagged(text);
	
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
	String tagged = remove_tag_contents(text, _("<atom-reminder"));
	tagged = remove_tag_contents(tagged, _("<atom-keyword")); // OLD, TODO: REMOVEME
	tagged = remove_tag_contents(tagged, _("<atom-kwpph>"));
	tagged = remove_tag(tagged, _("<keyword-param"));
	tagged = remove_tag(tagged, _("<param-"));
	String untagged = untag_no_escape(tagged);
	
	if (!root) return tagged;
	
	String result;
	
	// Find keywords
	while (!tagged.empty()) {
		vector<KeywordTrie*> current; // current location(s) in the trie
		vector<KeywordTrie*> next;    // location(s) after this step
		set<const Keyword*>  used;    // keywords already investigated
		current.push_back(root);
		closure(current);
		// is the keyword expanded? From <kw-?> tag
		// Possible values are:
		//  - '0' = reminder text explicitly hidden
		//  - '1' = reminder text explicitly shown
		//  - 'a' = reminder text in default state, hidden
		//  - 'A' = reminder text in default state, shown
		const char default_expand_type = 'a';
		char expand_type = default_expand_type;
		
		for (size_t i = 0 ; i < tagged.size() ;) {
			Char c = tagged.GetChar(i);
			// tag?
			if (c == _('<')) {
				if (is_substr(tagged, i, _("<kw-")) && i + 4 < tagged.size()) {
					expand_type = tagged.GetChar(i + 4); // <kw-?>
					tagged = tagged.erase(i, skip_tag(tagged,i)-i); // remove the tag from the string
				} else if (is_substr(tagged, i, _("</kw-"))) {
					expand_type = default_expand_type;
					tagged = tagged.erase(i, skip_tag(tagged,i)-i); // remove the tag from the string
				} else if (is_substr(tagged, i, _("<atom"))) {
					i = match_close_tag_end(tagged, i); // skip <atom>s
				} else {
					i = skip_tag(tagged, i);
				}
				continue;
			} else {
				#if USE_CASE_INSENSITIVE_KEYWORDS
					c = toLower(c); // case insensitive matching
				#endif
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
			for (int set_or_game = 0 ; set_or_game <= 1 ; ++set_or_game) {
				FOR_EACH(n, current) {
					FOR_EACH(kw, n->finished) {
						if (kw->fixed != (bool)set_or_game) {
							continue; // first try set keywords, try game keywords in the second round
						}
						if (!used.insert(kw).second) {
							continue; // already seen this keyword
						}
						// we have found a possible match, for a keyword which we have not seen before
						if (tryExpand(*kw, i, tagged, untagged, result, expand_type,
						              match_condition, expand_default, combine_script, ctx,
						              stat, stat_key))
						{
							// it matches
							goto matched_keyword;
						}
					}
				}
			}
		}
		// Remainder of the string
		result += tagged;
		tagged.clear();
		
		matched_keyword:;
	}
	
	assert_tagged(result);
	return result;
}

bool KeywordDatabase::tryExpand(const Keyword& kw,
                                size_t expand_type_known_upto,
                                String& tagged,
                                String& untagged,
                                String& result,
                                char expand_type,
                                const ScriptValueP& match_condition,
                                const ScriptValueP& expand_default,
                                const ScriptValueP& combine_script,
                                Context& ctx,
                                KeywordUsageStatistics* stat,
                                Value* stat_key) const
{
	// try to match regex against the *untagged* string
	assert(!kw.match_re.empty());
	Regex::Results match;
	if (!kw.match_re.matches(match, untagged)) return false;
	
	// Find match position
	size_t start_u = match.position();
	size_t len_u   = match.length();
	size_t start = untagged_to_index(tagged, start_u, true),
	       end   = untagged_to_index(tagged, start_u + len_u, false);
	if (start == end) return false; // don't match empty keywords
	
	// a part of tagged has not been searched for <kw- tags
	// this can happen when the trie incorrectly matches too early
	for (size_t j = expand_type_known_upto+1 ; j < start ;) {
		Char c = tagged.GetChar(j);
		if (c == _('<')) {
			if (is_substr(tagged, j, _("<kw-")) && j + 4 < tagged.size()) {
				expand_type = tagged.GetChar(j + 4); // <kw-?>
			} else if (is_substr(tagged, j, _("</kw-"))) {
				expand_type = 'a';
			}
			j = skip_tag(tagged, j);
		} else {
			++j;
		}
	}
	
	// To determine if the case matches exactly we compare plain text parts with the original match string
	size_t pos_in_match_string = 0;
	bool correct_case = true;
	// also check if there are missing parameters
	bool used_placeholders = false;
	
	
	// Split the keyword, set parameters in context
	// The even captures are parameter values, the odd ones are the plain text in between
	String total; // the total keyword
	assert(match.size() - 1 == 1 + 2 * kw.parameters.size());
	size_t part_start = start;
	for (size_t submatch = 1 ; submatch < match.size() ; ++submatch) {
		// the matched part
		size_t part_start_u = match.position(submatch);
		size_t part_len_u   = match.length((int)submatch);
		size_t part_end_u   = part_start_u + part_len_u;
		// note: start_u can be (uint)-1 when part_len_u == 0
		size_t part_end = part_len_u > 0 ? untagged_to_index(tagged, part_end_u, false) : part_start;
		String part(tagged, part_start, part_end - part_start);
		// strip left over </kw tags
		part = remove_tag(part,_("</kw-"));
		
		// we start counting at 1, so
		// submatch = 1 mod 2 -> text
		// submatch = 0 mod 2 -> parameter
		if ((submatch % 2) == 0) {
			// parameter
			KeywordParam& kwp = *kw.parameters[(submatch - 2) / 2];
			String param = untagged.substr(part_start_u, part_len_u); // untagged version
			// strip separator_before
			String separator_before, separator_after;
			Regex::Results sep_match;
			if (!kwp.separator_before_re.empty() && kwp.separator_before_re.matches(sep_match, param)) {
				size_t sep_end = sep_match.length();
				assert(sep_match.position() == 0); // should only match at start of param
				separator_before.assign(param, 0, sep_end);
				param.erase(0, sep_end);
				// strip from tagged version
				size_t sep_end_t = untagged_to_index(part, sep_end, false);
				part = get_tags(part, 0, sep_end_t, true, true) + part.substr(sep_end_t);
				// transform?
				if (kwp.separator_script) {
					ctx.setVariable(_("input"), to_script(separator_before));
					separator_before = kwp.separator_script.invoke(ctx)->toString();
				}
			}
			// strip separator_after
			if (!kwp.separator_after_re.empty() && kwp.separator_after_re.matches(sep_match, param)) {
				size_t sep_start = sep_match.position();
				assert(sep_match[0].second == param.end()); // should only match at end of param
				separator_after.assign(param, sep_start, String::npos);
				param.resize(sep_start);
				// strip from tagged version
				size_t sep_start_t = untagged_to_index(part, sep_start, false);
				part = part.substr(0, sep_start_t) + get_tags(part, sep_start_t, part.size(), true, true);
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
				used_placeholders = true;
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
			ctx.setVariable(String(_("param")) << (int)(submatch/2), script_param);
			
		} else if (correct_case) {
			// Plain text, check if the case matches
			for (size_t i = part_start_u ; i < part_start_u + part_len_u ; ++i, ++pos_in_match_string) {
				if (pos_in_match_string > kw.match.size()) {
					// outside match string, shouldn't happen, strings should be the same length
					correct_case = false;
					break;
				}
				Char actual_char = untagged.GetChar(i);
				Char match_char  = kw.match.GetChar(pos_in_match_string);
				if (actual_char != match_char) {
					correct_case = false;
					break;
				}
			}
			// we should have arrived at a param tag, skip it
			if (pos_in_match_string < kw.match.size() && is_substr(kw.match, pos_in_match_string, _("<atom-param"))) {
				pos_in_match_string = match_close_tag_end(kw.match, pos_in_match_string);
			}
		}
		
		total += part;
		part_start = part_end;
	}
	ctx.setVariable(_("mode"), to_script(kw.mode));
	ctx.setVariable(_("correct case"), to_script(correct_case));
	ctx.setVariable(_("used placeholders"), to_script(used_placeholders));
	
	// Final check whether the keyword matches
	if (match_condition && (bool)*match_condition->eval(ctx) == false) {
		return false;
	}
	
	// Show reminder text?
	bool expand = expand_type == _('1');
	if (!expand && expand_type != _('0')) {
		// default expand, determined by script
		expand = expand_default ? (bool)*expand_default->eval(ctx) : true;
		expand_type = expand ? _('A') : _('a');
	}
	
	// Copy text before keyword
	result += remove_tag(tagged.substr(0, start), _("<kw-"));
	
	// Combine keyword & reminder with result
	String reminder;
	try {
		reminder = kw.reminder.invoke(ctx)->toString();
	} catch (const Error& e) {
		handle_error(_ERROR_2_("in keyword reminder", e.what(), kw.keyword));
	}
	ctx.setVariable(_("keyword"),  to_script(total));
	ctx.setVariable(_("reminder"), to_script(reminder));
	ctx.setVariable(_("expand"),   to_script(expand));
	result +=  _("<kw-"); result += expand_type; result += _(">");
	result += combine_script->eval(ctx)->toString();
	result += _("</kw-"); result += expand_type; result += _(">");
	
	// Add to usage statistics
	if (stat && stat_key) {
		stat->push_back(make_pair(stat_key, &kw));
	}
	
	// After keyword
	tagged   = tagged.substr(end);
	untagged = untagged.substr(start_u + len_u);
	
	return true;
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
KeywordParamValue::operator bool()   const { return *to_script(value); }
KeywordParamValue::operator AColor() const { return *to_script(value); }
int KeywordParamValue::itemCount()   const { return  to_script(value)->itemCount(); }

ScriptValueP KeywordParamValue::getMember(const String& name) const {
	if (name == _("type"))             return to_script(type_name);
	if (name == _("separator before")) return to_script(separator_before);
	if (name == _("separator after"))  return to_script(separator_after);
	if (name == _("value"))            return to_script(value);
	if (name == _("param"))            return to_script(value);
	return ScriptValue::getMember(name);
}
