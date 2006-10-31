//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <script/context.hpp>
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
		throw "TODO";
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
	ScriptValueP replace = ctx.getVariable(_("replace"));
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


class ScriptSortRule : public ScriptValue {
  public:
	ScriptSortRule(const String& order) : order(order) {}
	
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("sort_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM(String, input);
		SCRIPT_RETURN(spec_sort(order, input));
	}
	
  private:
	String order;
};

// Create a rule for spec_sorting strings
SCRIPT_FUNCTION(sort_rule) {
	SCRIPT_PARAM(String, order);
	return new_intrusive1<ScriptSortRule>(order);
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

// ----------------------------------------------------------------------------- : Vector stuff

/// position of some element in a vector
/** 1 based index, 0 if not found */
int position_in_vector(const ScriptValueP& of, const ScriptValueP& in, const ScriptValueP& order_by) {
	return 0;// TODO
}

// finding positions
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
	ctx.setVariable(_("replace rule"),    script_replace_rule);
	ctx.setVariable(_("sort rule"),       script_sort_rule);
	ctx.setVariable(_("to upper"),        script_to_upper);
	ctx.setVariable(_("to lower"),        script_to_lower);
	ctx.setVariable(_("to title"),        script_to_title);
	ctx.setVariable(_("substring"),       script_substring);
	ctx.setVariable(_("position"),        script_position_of);
	ctx.setVariable(_("number of items"), script_number_of_items);
}

