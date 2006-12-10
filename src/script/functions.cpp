//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <script/context.hpp>
#include <util/tagged_string.hpp>
#include <data/set.hpp>
#include <wx/regex.h>

DECLARE_TYPEOF_COLLECTION(UInt);

/** @file script/functions.cpp
 *
 *  @brief Functions used in scripts
 */

// ----------------------------------------------------------------------------- : Rules : regex replace

class ScriptReplaceRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("replace_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(String, input);
		if (context.IsValid() || replacement_function) {
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
							ctx.setVariable(name, toScript(value));
						}
						// call
						inside = (String)*replacement_function->eval(ctx);
					} else {
						regex.Replace(&inside, replacement, 1); // replace inside
					}
				}
				ret  += inside;
				input = next_input;
			}
			ret += input;
			SCRIPT_RETURN(ret);
		} else {
			// dumb replacing
			regex.Replace(&input, replacement);
			SCRIPT_RETURN(input);
		}
	}
	
	wxRegEx      regex;					///< Regex to match
	wxRegEx      context;				///< Match only in a given context, optional
	String       replacement;			///< Replacement
	ScriptValueP replacement_function;	///< Replacement function instead of a simple string, optional
};

// Create a regular expression rule for replacing in strings
SCRIPT_FUNCTION(replace_rule) {
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
		ret->replacement = (String)*replace;
	}
	// in_context
	SCRIPT_OPTIONAL_PARAM_N(String, "in context", in_context) {
		if (!ret->context.Compile(in_context, wxRE_ADVANCED)) {
			throw ScriptError(_("Error while compiling regular expression: '")+in_context+_("'"));
		}
	}
	return ret;
}

// ----------------------------------------------------------------------------- : Rules : regex filter

class ScriptFilterRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("replace_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(String, input);
		String ret;
		while (regex.Matches(input)) {
			// match, append to result
			size_t start, len;
			bool ok = regex.GetMatch(&start, &len, 0);
			assert(ok);
			ret  += input.substr(start, len);  // the match
			input = input.substr(start + len); // everything after the match
		}
		SCRIPT_RETURN(ret);
	}
	
	wxRegEx regex; ///< Regex to match
};

// Create a regular expression rule for filtering strings
SCRIPT_FUNCTION(filter_rule) {
	intrusive_ptr<ScriptFilterRule> ret(new ScriptFilterRule);
	// match
	SCRIPT_PARAM(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	return ret;
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
 *     "WUR")      -> "RWU"      // by rule 1 could be "R WU" or "WU R", "RUW" has an earlier hole
 */
String spec_sort(const String& spec, const String& input) {
	String ret;
	for (size_t pos = 0 ; pos < spec.size() ; ++pos) {
		Char c = spec.GetChar(pos);
		
		if (c == _('<')) {			// keep only a single copy
			for ( ; pos < spec.size() ; ++pos) {
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
			size_t end = spec.find_first_of(_(')'));
			if (end == String::npos) throw ParseError(_("Expected ')' in sort_rule specification"));
			ret += cycle_sort(spec.substr(pos, end - pos - 1), input);
			pos = end;
		
		} else {					// single char
			FOR_EACH_CONST(d, input) {
				if (c == d) ret += c;
			}
		}
	}
	return ret;
}


// Utility for defining a script rule with a single parameter
#define SCRIPT_RULE_1(funname, type1, name1)								\
	class ScriptRule_##funname: public ScriptValue {						\
	  public:																\
		inline ScriptRule_##funname(const type1& name1) : name1(name1) {}	\
		virtual ScriptType type() const { return SCRIPT_FUNCTION; }			\
		virtual String typeName() const { return _(#funname)_("_rule"); }	\
		virtual ScriptValueP eval(Context& ctx) const;						\
	  private:																\
		type1 name1;														\
	};																		\
	SCRIPT_FUNCTION(funname##_rule) {										\
		SCRIPT_PARAM(type1, name1);											\
		return new_intrusive1<ScriptRule_##funname>(name1);					\
	}																		\
	SCRIPT_FUNCTION(funname) {												\
		SCRIPT_PARAM(type1, name1);											\
		return ScriptRule_##funname(name1).eval(ctx);						\
	}																		\
	ScriptValueP ScriptRule_##funname::eval(Context& ctx) const

// Utility for defining a script rule with two parameters
#define SCRIPT_RULE_2(funname, type1, name1, type2, name2)					\
	class ScriptRule_##funname: public ScriptValue {						\
	  public:																\
		inline ScriptRule_##funname(const type1& name1, const type2& name2)	\
			: name1(name1), name2(name2) {}									\
		virtual ScriptType type() const { return SCRIPT_FUNCTION; }			\
		virtual String typeName() const { return _(#funname)_("_rule"); }	\
		virtual ScriptValueP eval(Context& ctx) const;						\
	  private:																\
		type1 name1;														\
		type2 name2;														\
	};																		\
	SCRIPT_FUNCTION(funname##_rule) {										\
		SCRIPT_PARAM(type1, name1);											\
		SCRIPT_PARAM(type2, name2);											\
		return new_intrusive2<ScriptRule_##funname>(name1, name2);			\
	}																		\
	SCRIPT_FUNCTION(funname) {												\
		SCRIPT_PARAM(type1, name1);											\
		SCRIPT_PARAM(type2, name2);											\
		return ScriptRule_##funname(name1, name2).eval(ctx);				\
	}																		\
	ScriptValueP ScriptRule_##funname::eval(Context& ctx) const


// Create a rule for spec_sorting strings
SCRIPT_RULE_1(sort, String, order) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(spec_sort(order, input));
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

// ----------------------------------------------------------------------------- : Tagged stuff

String replace_tag_contents(String input, const String& tag, const ScriptValueP& contents, Context& ctx) {
	String ret;
	size_t pos = input.find(tag);
	while (pos != String::npos) {
		// find end of tag and contents
		size_t end = match_close_tag(input, pos);
		if (end == String::npos) break; // missing close tag
		// prepare for call
		String old_contents = input.substr(pos + tag.size(), end - (pos + tag.size()));
		ctx.setVariable(_("contents"), toScript(old_contents));
		// replace
		ret += input.substr(0, pos); // before tag
		ret += tag;
		ret += *contents->eval(ctx);// new contents (call)
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

// ----------------------------------------------------------------------------- : Vector stuff

/// compare script values for equallity
bool equal(const ScriptValue& a, const ScriptValue& b) {
	ScriptType at = a.type(), bt = b.type();
	if (at != bt) {
		return false;
	} else if (at == SCRIPT_INT) {
		return    (int)a == (int)b;
	} else if (at == SCRIPT_DOUBLE) {
		return (double)a == (double)b;
	} else if (at == SCRIPT_STRING) {
		return (String)a == (String)b;
	} else if (at == SCRIPT_OBJECT) {
		// HACK for ScriptObject<shared_ptr<X> >
		// assumes different types are layed out the same, and that
		// should be void*, but then we need getMember for void
		const ScriptObject<int*>& av = reinterpret_cast<const ScriptObject<int*>&>(a);
		const ScriptObject<int*>& bv = reinterpret_cast<const ScriptObject<int*>&>(b);
		return av.getValue() == bv.getValue();
	}
	return &a == &b;
}

/// position of some element in a vector
/** 0 based index, -1 if not found */
int position_in_vector(const ScriptValueP& of, const ScriptValueP& in, const ScriptValueP& order_by) {
	ScriptType of_t = of->type(), in_t = in->type();
	if (of_t == SCRIPT_STRING || in_t == SCRIPT_STRING) {
		// string finding
		return (int)((String)*of).find(*in); // (int)npos == -1
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
		ScriptValueP it = in->makeIterator();
		int i = 0;
		while (ScriptValueP v = it->next()) {
			if (equal(*of, *v)) return i;
			i++;
		}
	}
	return -1; // TODO?
}

// finding positions, also of substrings
SCRIPT_FUNCTION(position_of) {
	ScriptValueP of       = ctx.getVariable(_("of"));
	ScriptValueP in       = ctx.getVariable(_("in"));
	ScriptValueP order_by = ctx.getVariableOpt(_("order by"));
	SCRIPT_RETURN(position_in_vector(of, in, order_by));
}

// finding sizes
SCRIPT_FUNCTION(number_of_items) {
	SCRIPT_RETURN(ctx.getVariable(_("in"))->itemCount());
}

// ----------------------------------------------------------------------------- : Initialize functions

void init_script_functions(Context& ctx) {
	ctx.setVariable(_("replace rule"),      script_replace_rule);
	ctx.setVariable(_("filter rule"),       script_filter_rule);
	ctx.setVariable(_("sort"),              script_sort);
	ctx.setVariable(_("sort rule"),         script_sort_rule);
	ctx.setVariable(_("to upper"),          script_to_upper);
	ctx.setVariable(_("to lower"),          script_to_lower);
	ctx.setVariable(_("to title"),          script_to_title);
	ctx.setVariable(_("substring"),         script_substring);
	ctx.setVariable(_("contains"),          script_contains);
	ctx.setVariable(_("tag contents"),      script_tag_contents);
	ctx.setVariable(_("remove tag"),        script_tag_remove);
	ctx.setVariable(_("tag contents rule"), script_tag_contents_rule);
	ctx.setVariable(_("tag remove rule"),   script_tag_remove_rule);
	ctx.setVariable(_("position"),          script_position_of);
	ctx.setVariable(_("number of items"),   script_number_of_items);
}

