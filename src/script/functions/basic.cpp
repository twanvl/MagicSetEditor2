
//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/tagged_string.hpp>
#include <util/error.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/game.hpp>

DECLARE_TYPEOF_COLLECTION(pair<String COMMA ScriptValueP>);

// ----------------------------------------------------------------------------- : Debugging

SCRIPT_FUNCTION(trace) {
	SCRIPT_PARAM(String, input);
	//handle_warning(_("Trace:\t") + input, false);
	wxLogDebug(_("Trace:\t") + input);
	SCRIPT_RETURN(input);
}

// ----------------------------------------------------------------------------- : String stuff

// convert a string to upper case
SCRIPT_FUNCTION(to_upper) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(input.Upper());
}

// convert a string to lower case
SCRIPT_FUNCTION(to_lower) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(input.Lower());
}

// convert a string to title case
SCRIPT_FUNCTION(to_title) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(capitalize(input));
}

// reverse a string
SCRIPT_FUNCTION(reverse) {
	SCRIPT_PARAM(String, input);
	reverse(input.begin(), input.end());
	SCRIPT_RETURN(input);
}

// extract a substring
SCRIPT_FUNCTION(substring) {
	SCRIPT_PARAM(String, input);
	SCRIPT_PARAM_DEFAULT(int, begin, 0);
	SCRIPT_PARAM_DEFAULT(int, end,   INT_MAX);
	if (begin < 0) begin = 0;
	if (end   < 0) end   = 0;
	if (begin >= end || (size_t)begin >= input.size()) {
		SCRIPT_RETURN(wxEmptyString);
	} else if ((size_t)end >= input.size()) {
		SCRIPT_RETURN(input.substr(begin));
	} else {
		SCRIPT_RETURN(input.substr(begin, end - begin));
	}
}

// does a string contain a substring?
SCRIPT_FUNCTION(contains) {
	SCRIPT_PARAM(String, input);
	SCRIPT_PARAM(String, match);
	SCRIPT_RETURN(input.find(match) != String::npos);
}

SCRIPT_RULE_1(format, String, format) {
	String fmt = _("%") + replace_all(format, _("%"), _(""));
	// determine type expected by format string
	if (format.find_first_of(_("DdIiOoXx")) != String::npos) {
		SCRIPT_PARAM(int, input);
		SCRIPT_RETURN(String::Format(fmt, input));
	} else if (format.find_first_of(_("EeFfGg")) != String::npos) {
		SCRIPT_PARAM(double, input);
		SCRIPT_RETURN(String::Format(fmt, input));
	} else if (format.find_first_of(_("Ss")) != String::npos) {
		SCRIPT_PARAM(String, input);
		SCRIPT_RETURN(format_string(fmt, input));
	} else {
		throw ScriptError(_ERROR_1_("unsupported format", format));
	}
}

SCRIPT_FUNCTION(curly_quotes) {
	SCRIPT_PARAM(String, input);
	bool open = true, in_tag = false;
	FOR_EACH(c, input) {
		if (c == _('\'') || c == LEFT_SINGLE_QUOTE || c == RIGHT_SINGLE_QUOTE) {
			c = open ? LEFT_SINGLE_QUOTE : RIGHT_SINGLE_QUOTE;
		} else if (c == _('\"') || c == LEFT_DOUBLE_QUOTE || c == RIGHT_DOUBLE_QUOTE) {
			c = open ? LEFT_DOUBLE_QUOTE : RIGHT_DOUBLE_QUOTE;
		} else if (c == _('<')) {
			in_tag = true;
		} else if (c == _('>')) {
			in_tag = false;
		} else if (!in_tag) {
			// Also allow double-nesting of quotes
			open = isSpace(c) || c == _('(') || c == _('[');
		}
	}
	SCRIPT_RETURN(input);
}


// ----------------------------------------------------------------------------- : Tagged string

/// Replace the contents of a specific tag with the value of a script function
String replace_tag_contents(String input, const String& tag, const ScriptValueP& contents, Context& ctx) {
	String ret;
	size_t pos = input.find(tag);
	while (pos != String::npos) {
		// find end of tag and contents
		size_t end = match_close_tag(input, pos);
		if (end == String::npos) break; // missing close tag
		// prepare for call
		String old_contents = input.substr(pos + tag.size(), end - (pos + tag.size()));
		ctx.setVariable(_("contents"), to_script(old_contents));
		// replace
		ret += input.substr(0, pos); // before tag
		ret += tag;
		ret += contents->eval(ctx)->toString();// new contents (call)
		ret += close_tag(tag);
		// next
		input = input.substr(skip_tag(input,end));
		pos = input.find(tag);
	}
	return ret + input;
}

// Replace the contents of a specific tag
SCRIPT_RULE_2(tag_contents,  String, tag,  ScriptValueP, contents) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(replace_tag_contents(input, tag, contents, ctx));
}

SCRIPT_RULE_1(tag_remove, String, tag) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(remove_tag(input, tag));
}

// ----------------------------------------------------------------------------- : Collection stuff

/// compare script values for equallity
bool equal(const ScriptValue& a, const ScriptValue& b) {
	if (&a == &b) return true;
	ScriptType at = a.type(), bt = b.type();
	if (at != bt) {
		return false;
	} else if (at == SCRIPT_INT) {
		return    (int)a == (int)b;
	} else if (at == SCRIPT_DOUBLE) {
		return (double)a == (double)b;
	} else if (at == SCRIPT_STRING) {
		return a.toString() == b.toString();
	} else {
		// compare pointers, must be equal and not null
		const void *ap = a.comparePointer(), *bp = b.comparePointer();
		return (ap && ap == bp);
	}
}

/// position of some element in a vector
/** 0 based index, -1 if not found */
int position_in_vector(const ScriptValueP& of, const ScriptValueP& in, const ScriptValueP& order_by) {
	ScriptType of_t = of->type(), in_t = in->type();
	if (of_t == SCRIPT_STRING || in_t == SCRIPT_STRING) {
		// string finding
		return (int)of->toString().find(in->toString()); // (int)npos == -1
	} else if (order_by) {
		ScriptObject<Set*>*  s = dynamic_cast<ScriptObject<Set*>* >(in.get());
		ScriptObject<CardP>* c = dynamic_cast<ScriptObject<CardP>*>(of.get());
		if (s && c) {
			return s->getValue()->positionOfCard(c->getValue(), order_by);
		} else {
			throw ScriptError(_("position: using 'order_by' is only supported for finding cards in the set"));
		}
	} else {
		// unordered position
		ScriptValueP it = in->makeIterator(in);
		int i = 0;
		while (ScriptValueP v = it->next()) {
			if (equal(*of, *v)) return i;
			i++;
		}
	}
	return -1; // TODO?
}

// sort a script list
ScriptValueP sort_script(Context& ctx, const ScriptValueP& list, ScriptValue& order_by) {
	ScriptType list_t = list->type();
	if (list_t == SCRIPT_STRING) {
		// sort a string
		String s = list->toString();
		sort(s.begin(), s.end());
		SCRIPT_RETURN(s);
	} else {
		// are we sorting a set?
		ScriptObject<Set*>* set = dynamic_cast<ScriptObject<Set*>*>(list.get());
		// sort a collection
		vector<pair<String,ScriptValueP> > values;
		ScriptValueP it = list->makeIterator(list);
		while (ScriptValueP v = it->next()) {
			ctx.setVariable(set ? _("card") : _("input"), v);
			values.push_back(make_pair(order_by.eval(ctx)->toString(), v));
		}
		sort(values.begin(), values.end());
		// return collection
		intrusive_ptr<ScriptCustomCollection> ret(new ScriptCustomCollection());
		FOR_EACH(v, values) {
			ret->value.push_back(v.second);
		}
		return ret;
	}
}

// finding positions, also of substrings
SCRIPT_FUNCTION_WITH_DEP(position_of) {
	ScriptValueP of       = ctx.getVariable(_("of"));
	ScriptValueP in       = ctx.getVariable(_("in"));
	ScriptValueP order_by = ctx.getVariableOpt(_("order by"));
	SCRIPT_RETURN(position_in_vector(of, in, order_by));
}
SCRIPT_FUNCTION_DEPENDENCIES(position_of) {
	ScriptValueP of       = ctx.getVariable(_("of"));
	ScriptValueP in       = ctx.getVariable(_("in"));
	ScriptValueP order_by = ctx.getVariableOpt(_("order by"));
	ScriptObject<Set*>*  s = dynamic_cast<ScriptObject<Set*>* >(in.get());
	ScriptObject<CardP>* c = dynamic_cast<ScriptObject<CardP>*>(of.get());
	if (s && c) {
		// dependency on cards
		mark_dependency_member(*s->getValue(), _("cards"), dep);
		if (order_by) {
			// dependency on order_by function
			order_by->dependencies(ctx, dep.makeCardIndependend());
		}
	}
	return dependency_dummy;
};

// finding sizes
SCRIPT_FUNCTION(number_of_items) {
	SCRIPT_RETURN(ctx.getVariable(_("in"))->itemCount());
}

// filtering items from a list
SCRIPT_FUNCTION(filter_list) {
	SCRIPT_PARAM(ScriptValueP, input);
	SCRIPT_PARAM(ScriptValueP, filter);
	// filter a collection
	intrusive_ptr<ScriptCustomCollection> ret(new ScriptCustomCollection());
	ScriptValueP it = input->makeIterator(input);
	while (ScriptValueP v = it->next()) {
		ctx.setVariable(_("input"), v);
		if (*filter->eval(ctx)) {
			ret->value.push_back(v);
		}
	}
	// TODO : somehow preserve keys
	return ret;
}

// ----------------------------------------------------------------------------- : Keywords

SCRIPT_RULE_2_N_DEP(expand_keywords, ScriptValueP, _("default expand"), default_expand,
                                     ScriptValueP, _("combine"),        combine) {
	SCRIPT_PARAM(String, input);
	SCRIPT_PARAM(Set*, set);
	KeywordDatabase& db = set->keyword_db;
	if (db.empty()) {
		db.prepare_parameters(set->game->keyword_parameter_types, set->game->keywords);
		db.prepare_parameters(set->game->keyword_parameter_types, set->keywords);
		db.add(set->game->keywords);
		db.add(set->keywords);
	}
	SCRIPT_OPTIONAL_PARAM_(CardP, card);
	WITH_DYNAMIC_ARG(keyword_usage_statistics, card ? &card->keyword_usage : nullptr);
	SCRIPT_RETURN(db.expand(input, default_expand, combine, ctx));
}
SCRIPT_RULE_2_DEPENDENCIES(expand_keywords) {
	default_expand->dependencies(ctx, dep);
	combine       ->dependencies(ctx, dep);
	SCRIPT_PARAM(Set*, set);
	set->game->dependent_scripts_keywords.add(dep); // this depends on the set's keywords
	SCRIPT_RETURN(_(""));
}

// ----------------------------------------------------------------------------- : Rules : regex replace

class ScriptReplaceRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("replace_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(String, input);
		if (context.IsValid() || replacement_function || recursive) {
			SCRIPT_RETURN(apply(ctx, input));
		} else {
			// dumb replacing
			regex.Replace(&input, replacement);
			SCRIPT_RETURN(input);
		}
	}
	String apply(Context& ctx, String& input, int level = 0) const {
		// match first, then check context of match
		String ret;
		while (regex.Matches(input)) {
			// for each match ...
			size_t start, len;
			bool ok = regex.GetMatch(&start, &len, 0);
			assert(ok);
			ret                 += input.substr(0, start);          // everything before the match position stays
			String inside        = input.substr(start, len);        // inside the match
			String next_input    = input.substr(start + len);       // next loop the input is after this match
			String after_replace = ret + _("<match>") + next_input; // after replacing, the resulting context would be
			if (!context.IsValid() || context.Matches(after_replace)) {
				// the context matches -> perform replacement
				if (replacement_function) {
					// set match results in context
					for (UInt m = 0 ; m < regex.GetMatchCount() ; ++m) {
						regex.GetMatch(&start, &len, m);
						String name  = m == 0 ? _("input") : String(_("_")) << m;
						String value = input.substr(start, len);
						ctx.setVariable(name, to_script(value));
					}
					// call
					inside = replacement_function->eval(ctx)->toString();
				} else {
					regex.Replace(&inside, replacement, 1); // replace inside
				}
			}
			if (recursive && level < 20) {
				ret += apply(ctx, inside, level + 1);
			} else {
				ret += inside;
			}
			input = next_input;
		}
		ret += input;
		return ret;
	}
	
	wxRegEx      regex;					///< Regex to match
	wxRegEx      context;				///< Match only in a given context, optional
	String       replacement;			///< Replacement
	ScriptValueP replacement_function;	///< Replacement function instead of a simple string, optional
	bool         recursive;				///< Recurse into the replacement
};

// Create a regular expression rule for replacing in strings
ScriptValueP replace_rule(Context& ctx) {
	intrusive_ptr<ScriptReplaceRule> ret(new ScriptReplaceRule);
	// match
	SCRIPT_PARAM(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	// replace
	SCRIPT_PARAM(ScriptValueP, replace);
	if (replace->type() == SCRIPT_FUNCTION) {
		ret->replacement_function = replace;
	} else {
		ret->replacement = replace->toString();
	}
	// in_context
	SCRIPT_OPTIONAL_PARAM_N(String, _("in context"), in_context) {
		if (!ret->context.Compile(in_context, wxRE_ADVANCED)) {
			throw ScriptError(_("Error while compiling regular expression: '")+in_context+_("'"));
		}
	}
	SCRIPT_OPTIONAL_PARAM_(bool, recursive);
	ret->recursive = recursive;
	return ret;
}

SCRIPT_FUNCTION(replace_rule) {
	return replace_rule(ctx);
}
SCRIPT_FUNCTION(replace) {
	return replace_rule(ctx)->eval(ctx);
}

// ----------------------------------------------------------------------------- : Rules : regex filter

class ScriptFilterRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("filter_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(String, input);
		String ret;
		while (regex.Matches(input)) {
			// match, append to result
			size_t start, len;
			bool ok = regex.GetMatch(&start, &len, 0);
			assert(ok);
			String inside     = input.substr(start, len);  // the match
			String next_input =  input.substr(start + len); // everything after the match
			if (!context.IsValid() || context.Matches(input.substr(0,start) + _("<match>") + next_input)) {
				// no context or context match
				ret += inside;
			}
			input = next_input;
		}
		SCRIPT_RETURN(ret);
	}
	
	wxRegEx regex;   ///< Regex to match
	wxRegEx context; ///< Match only in a given context, optional
};

// Create a regular expression rule for filtering strings
ScriptValueP filter_rule(Context& ctx) {
	intrusive_ptr<ScriptFilterRule> ret(new ScriptFilterRule);
	// match
	SCRIPT_PARAM(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	// in_context
	SCRIPT_OPTIONAL_PARAM_N(String, _("in context"), in_context) {
		if (!ret->context.Compile(in_context, wxRE_ADVANCED)) {
			throw ScriptError(_("Error while compiling regular expression: '")+in_context+_("'"));
		}
	}
	return ret;
}

SCRIPT_FUNCTION(filter_rule) {
	return filter_rule(ctx);
}
SCRIPT_FUNCTION(filter) {
	return filter_rule(ctx)->eval(ctx);
}

// ----------------------------------------------------------------------------- : Rules : regex match

class ScriptMatchRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("match_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(String, input);
		SCRIPT_RETURN(regex.Matches(input));
	}
	
	wxRegEx regex; ///< Regex to match
};

// Create a regular expression rule for filtering strings
ScriptValueP match_rule(Context& ctx) {
	intrusive_ptr<ScriptMatchRule> ret(new ScriptMatchRule);
	// match
	SCRIPT_PARAM(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	return ret;
}

SCRIPT_FUNCTION(match_rule) {
	return match_rule(ctx);
}
SCRIPT_FUNCTION(match) {
	return match_rule(ctx)->eval(ctx);
}

// ----------------------------------------------------------------------------- : Rules : sort

/// Sort a string using a specification using the shortest cycle metric, see spec_sort
String cycle_sort(const String& spec, const String& input) {
	size_t size = spec.size();
	vector<UInt> counts;
	// count occurences of each char in spec
	FOR_EACH_CONST(s, spec) {
		UInt c = 0;
		FOR_EACH_CONST(i, input) {
			if (s == i) c++;
		}
		counts.push_back(c);
	}
	// determine best start point
	size_t best_start = 0;
	UInt   best_start_score = 0xffffffff;
	for (size_t start = 0 ; start < size ; ++start) {
		// score of a start position, can be considered as:
		//  - count saturated to binary
		//  - rotated left by start
		//  - interpreted as a binary number, but without trailing 0s
		UInt score = 0, mul = 1;
		for (size_t i = 0 ; i < size ; ++i) {
			mul *= 2;
			if (counts[(start + i) % size]) {
				score = score * mul + 1;
				mul = 1;
			}
		}
		if (score < best_start_score) {
			best_start_score = score;
			best_start       = start;
		}
	}
	// return string
	String ret;
	for (size_t i = 0 ; i < size ; ++i) {
		size_t pos = (best_start + i) % size;
		ret.append(counts[pos], spec[pos]);
	}
	return ret;
}

/// Sort a string using a sort specification
/** The specificatio can contain:
 *   - a       = all 'a's go here
 *   - [abc]   = 'a', 'b' and 'c' go here, in the same order as in the input
 *   - <abc>   = 'a', 'b' and 'c' go here in that order, and only zero or one time.
 *   - (abc)   = 'a', 'b' and 'c' go here, in the shortest order
 *               consider the specified characters as a clockwise circle
 *               then returns the input in the order that:
 *                 1. takes the shortest clockwise path over this circle.
 *                 2. has _('holes') early, a hole means a character that is in the specification
 *                    but not in the input
 *                 3. prefer the one that comes the earliest in the expression (a in this case)
 *
 *  example:
 *    spec_sort("XYZ<0123456789>(WUBRG)",..)  // used by magic
 *     "W1G")      -> "1GW"      // could be "W...G" or "...GW", second is shorter
 *     "GRBUWWUG") -> "WWUUBRGG" // no difference by rule 1,2, could be "WUBRG", "UBRGW", etc.
 *                               // becomes _("WUBRG") by rule 3
 *     "WUR")      -> "RWU"      // by rule 1 could be "R WU" or "WU R", "RWU" has an earlier hole
 */
String spec_sort(const String& spec, const String& input) {
	String ret;
	for (size_t pos = 0 ; pos < spec.size() ; ++pos) {
		Char c = spec.GetChar(pos);
		
		if (c == _('<')) {			// keep only a single copy
			for ( ++pos ; pos < spec.size() ; ++pos) {
				Char c = spec.GetChar(pos);
				if (c == _('>')) break;
				if (input.find_first_of(c) != String::npos) {
					ret += c; // input contains c
				}
			}
			if (pos == String::npos) throw ParseError(_("Expected '>' in sort_rule specification"));
			
		} else if (c == _('[')) {	// in any order
			size_t end = spec.find_first_of(_(']'));
			if (end == String::npos) throw ParseError(_("Expected ']' in sort_rule specification"));
			FOR_EACH_CONST(d, input) {
				size_t in_spec = spec.find_first_of(d, pos);
				if (in_spec < end) {
					ret += d; // d is in the part between [ and ]
				}
			}
			pos = end;
			
		} else if (c == _('(')) {	// in a cycle
			size_t end = spec.find_first_of(_(')'), pos);
			if (end == String::npos) throw ParseError(_("Expected ')' in sort_rule specification"));
			ret += cycle_sort(spec.substr(pos + 1, end - pos - 1), input);
			pos = end;
		
		} else {					// single char
			FOR_EACH_CONST(d, input) {
				if (c == d) ret += c;
			}
		}
	}
	return ret;
}

// Sort using spec_sort
class ScriptRule_sort_order: public ScriptValue {
  public:
	inline ScriptRule_sort_order(const String& order) : order(order) {}
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("sort_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(ScriptValueP, input);
		if (input->type() == SCRIPT_COLLECTION) {
			handle_warning(_("Sorting a collection as a string, this is probably not intended, if it is use 'collection+\"\"' to force conversion"), false);
		}
		SCRIPT_RETURN(spec_sort(order, input->toString()));
	}
  private:
	String order;
};
// Sort using sort_script
class ScriptRule_sort_order_by: public ScriptValue {
  public:
	inline ScriptRule_sort_order_by(const ScriptValueP& order_by) : order_by(order_by) {}
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("sort_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(ScriptValueP, input);
		return sort_script(ctx, input, *order_by);
	}
  private:
	ScriptValueP order_by;
};
// Sort a string alphabetically
class ScriptRule_sort: public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("sort_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(ScriptValueP, input);
		if (input->type() == SCRIPT_COLLECTION) {
			handle_warning(_("Sorting a collection as a string, this is probably not intended, if it is use 'collection+\"\"' to force conversion"), false);
		}
		String input_str = input->toString();
		sort(input_str.begin(), input_str.end());
		SCRIPT_RETURN(input_str);
	}
  private:
	ScriptValueP order_by;
};

SCRIPT_FUNCTION(sort_rule) {
	SCRIPT_OPTIONAL_PARAM(String, order) {
		return new_intrusive1<ScriptRule_sort_order   >(order);
	}
	SCRIPT_OPTIONAL_PARAM_N(ScriptValueP, _("order by"), order_by) {
		return new_intrusive1<ScriptRule_sort_order_by>(order_by);
	} else {
		return new_intrusive <ScriptRule_sort         >();
	}
}
SCRIPT_FUNCTION(sort) {
	SCRIPT_OPTIONAL_PARAM(String, order) {
		return ScriptRule_sort_order   (order   ).eval(ctx);
	}
	SCRIPT_OPTIONAL_PARAM_N(ScriptValueP, _("order by"), order_by) {
		return ScriptRule_sort_order_by(order_by).eval(ctx);
	} else {
		return ScriptRule_sort         (        ).eval(ctx);
	}
}

// ----------------------------------------------------------------------------- : Init

void init_script_basic_functions(Context& ctx) {
	// debugging
	ctx.setVariable(_("trace"),                script_trace);
	// string
	ctx.setVariable(_("to upper"),             script_to_upper);
	ctx.setVariable(_("to lower"),             script_to_lower);
	ctx.setVariable(_("to title"),             script_to_title);
	ctx.setVariable(_("reverse"),              script_reverse);
	ctx.setVariable(_("substring"),            script_substring);
	ctx.setVariable(_("contains"),             script_contains);
	ctx.setVariable(_("format"),               script_format);
	ctx.setVariable(_("format rule"),          script_format_rule);
	ctx.setVariable(_("curly quotes"),         script_curly_quotes);
	// tagged string
	ctx.setVariable(_("tag contents"),         script_tag_contents);
	ctx.setVariable(_("remove tag"),           script_tag_remove);
	ctx.setVariable(_("tag contents rule"),    script_tag_contents_rule);
	ctx.setVariable(_("tag remove rule"),      script_tag_remove_rule);
	// collection
	ctx.setVariable(_("position"),             script_position_of);
	ctx.setVariable(_("number of items"),      script_number_of_items);
	ctx.setVariable(_("filter list"),          script_filter_list);
	// keyword
	ctx.setVariable(_("expand keywords"),      script_expand_keywords);
	ctx.setVariable(_("expand keywords rule"), script_expand_keywords_rule);
	// advanced string rules
	ctx.setVariable(_("replace"),              script_replace);
	ctx.setVariable(_("filter"),               script_filter);
	ctx.setVariable(_("match"),                script_match);
	ctx.setVariable(_("sort"),                 script_sort);
	ctx.setVariable(_("sort list"),            script_sort);
	ctx.setVariable(_("replace rule"),         script_replace_rule);
	ctx.setVariable(_("filter rule"),          script_filter_rule);
	ctx.setVariable(_("match rule"),           script_match_rule);
	ctx.setVariable(_("sort rule"),            script_sort_rule);
}
