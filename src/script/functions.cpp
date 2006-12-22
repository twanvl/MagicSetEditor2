//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <script/context.hpp>
#include <script/dependency.hpp>
#include <util/tagged_string.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/field/text.hpp>
#include <wx/regex.h>

DECLARE_TYPEOF_COLLECTION(UInt);
DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(TextValue*);
DECLARE_TYPEOF_COLLECTION(String);

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
	SCRIPT_OPTIONAL_PARAM_N(String, _("in context"), in_context) {
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
SCRIPT_FUNCTION_DEP(position_of) {
	ScriptValueP of       = ctx.getVariable(_("of"));
	ScriptValueP in       = ctx.getVariable(_("in"));
	ScriptValueP order_by = ctx.getVariableOpt(_("order by"));
	SCRIPT_RETURN(position_in_vector(of, in, order_by));
}

ScriptValueP ScriptBuildin_position_of::dependencies(Context& ctx, const Dependency& dep) const {
	ScriptValueP of       = ctx.getVariable(_("of"));
	ScriptValueP in       = ctx.getVariable(_("in"));
	ScriptValueP order_by = ctx.getVariableOpt(_("order by"));
	ScriptObject<Set*>*  s = dynamic_cast<ScriptObject<Set*>* >(in.get());
	ScriptObject<CardP>* c = dynamic_cast<ScriptObject<CardP>*>(of.get());
	if (s && c) {
		// dependency on cards
		mark_dependency_member(s->getValue(), _("cards"), dep);
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


// ----------------------------------------------------------------------------- : Combined editor

SCRIPT_FUNCTION_DEP(combined_editor) {
	// read 'field#' arguments
	vector<TextValue*> values;
	for (int i = 0 ; ; ++i) {
		String name = _("field"); if (i > 0) name = name << i;
		SCRIPT_OPTIONAL_PARAM_N(ValueP, name, value) {
			TextValue* text_value = dynamic_cast<TextValue*>(value.get());
			if (!text_value) throw ScriptError(_("Argument '")+name+_("' should be a text field")); 
			values.push_back(text_value);
		} else if (i > 0) break;
	}
	if (values.empty()) {
		throw ScriptError(_("No fields specified for combined_editor"));
	}
	// read 'separator#' arguments
	vector<String> separators;
	for (int i = 0 ; ; ++i) {
		String name = _("separator"); if (i > 0) name = name << i;
		SCRIPT_OPTIONAL_PARAM_N(String, name, separator) {
			separators.push_back(separator);
		} else if (i > 0) break;
	}
	if (separators.size() < values.size() - 1) {
		throw ScriptError(String::Format(_("Not enough separators for combine_editor, expected %d"), values.size()-1));
	}
	// split the value
	SCRIPT_PARAM(String, value);
	vector<String> value_parts;
	size_t pos = value.find(_("<sep"));
	while (pos != String::npos) {
		value_parts.push_back(value.substr(0, pos));
		value = value.substr(min(skip_tag(value,match_close_tag(value,pos)), value.size()));
		pos = value.find(_("<sep"));
	}
	value_parts.push_back(value);
	if (value_parts.size() < values.size()) value_parts.resize(values.size());
	// update the values if our input value is newer?
	Age new_value_update = last_update_age();
	FOR_EACH_2(v, values, nv, value_parts) {
		if (v->value() != nv && v->last_update < new_value_update) {
			// TODO : type over
			v->value.assign(nv);
			v->update(ctx);
		}
		nv = v->value();
	}
	// options
	SCRIPT_PARAM_DEFAULT_N(bool, _("hide when empty"),   hide_when_empty,   false);
	SCRIPT_PARAM_DEFAULT_N(bool, _("soft before empty"), soft_before_empty, false);
	// recombine the parts
	String new_value = value_parts.front();
	for (size_t i = 1 ; i < value_parts.size() ; ++i) {
		if (value_parts[i].empty() && new_value.empty() && hide_when_empty) {
			// no separator
		} else if (value_parts[i].empty() && soft_before_empty) {
			// soft separator
			new_value += _("<sep-soft>") + separators[i - 1] + _("</sep-soft>");
		} else {
			// normal separator
			new_value += _("<sep>")      + separators[i - 1] + _("</sep>");
			new_value += value_parts[i];
		}
	}
	SCRIPT_RETURN(new_value);
}

ScriptValueP ScriptBuildin_combined_editor::dependencies(Context& ctx, const Dependency& dep) const {
	// read 'field#' arguments
	vector<FieldP> fields;
	for (int i = 0 ; ; ++i) {
		String name = _("field"); if (i > 0) name = name << i;
		SCRIPT_OPTIONAL_PARAM_N(ValueP, name, value) {
			fields.push_back(value->fieldP);
		} else if (i > 0) break;
	}
	// Find the target field
	SCRIPT_PARAM(Set*, set);
	GameP game = set->game;
	FieldP target_field;
	if      (dep.type == DEP_CARD_FIELD) target_field = game->card_fields[dep.index];
	else if (dep.type == DEP_SET_FIELD)  target_field = game->set_fields[dep.index];
	else                                 throw InternalError(_("Finding dependencies of combined error for non card/set field"));
	// Add dependencies, from target_field on field#
	// For card fields
	size_t j = 0;
	FOR_EACH(f, game->card_fields) {
		Dependency dep(DEP_CARD_COPY_DEP, j++);
		FOR_EACH(fn, fields) {
			if (f == fn) {
				target_field->dependent_scripts.add(dep);
				break;
			}
		}
	}
	// For set fields
	j = 0;
	FOR_EACH(f, game->set_fields) {
		Dependency dep(DEP_SET_COPY_DEP, j++);
		FOR_EACH(fn, fields) {
			if (f == fn) {
				target_field->dependent_scripts.add(dep);
				break;
			}
		}
	}
	SCRIPT_RETURN(dependency_dummy);
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
	ctx.setVariable(_("forward editor"),    script_combined_editor);
	ctx.setVariable(_("combined editor"),   script_combined_editor);
}

