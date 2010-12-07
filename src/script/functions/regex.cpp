//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/regex.hpp>
#include <util/error.hpp>

DECLARE_POINTER_TYPE(ScriptRegex);
DECLARE_TYPEOF_COLLECTION(pair<Variable COMMA ScriptValueP>);

// ----------------------------------------------------------------------------- : Regex type

/// A regular expression for use in a script
class ScriptRegex : public ScriptValue, public Regex {
  public:
	virtual ScriptType type() const { return SCRIPT_REGEX; }
	virtual String typeName() const { return _("regex"); }
	
	ScriptRegex(const String& code) {
		assign(code);
	}
	
	/// Match only if in_context also matches
	bool matches(Results& results, const String& str, String::const_iterator begin, const ScriptRegexP& in_context) {
		if (!in_context) {
			return matches(results, begin, str.end());
		} else {
			while (matches(results, begin, str.end())) {
				Results::const_reference match = results[0];
				String context_str(str.begin(), match.first); // before
				context_str += _("<match>");
				context_str.append(match.second, str.end()); // after
				if (in_context->matches(context_str)) {
					return true; // the context matches, done
				}
				begin = match.second; // skip
			}
			return false;
		}
	}
	using Regex::matches;
};

ScriptRegexP regex_from_script(const ScriptValueP& value) {
	// is it a regex already?
	ScriptRegexP regex = dynamic_pointer_cast<ScriptRegex>(value);
	if (!regex) {
		// TODO: introduce some kind of caching?
		regex = intrusive(new ScriptRegex(*value));
	}
	return regex;
}

template <> inline ScriptRegexP from_script<ScriptRegexP>(const ScriptValueP& value) {
	return regex_from_script(value);
}

// ----------------------------------------------------------------------------- : Rules : regex replace

struct RegexReplacer {
	ScriptRegexP match;					///< Regex to match
	ScriptRegexP context;				///< Match only in a given context, optional
	String       replacement_string;	///< Replacement
	ScriptValueP replacement_function;	///< Replacement function instead of a simple string, optional
	bool         recursive;				///< Recurse into the replacement
	
	String apply(Context& ctx, const String& input, int level = 0) const {
		String ret;
		String::const_iterator start = input.begin();
		ScriptRegex::Results results;
		while (match->matches(results, input, start, context)) {
			// for each match ...
			ScriptRegex::Results::const_reference pos = results[0];
			ret.append(start, pos.first); // everything before the match position stays
			// determine replacement
			String inside;
			if (replacement_function) {
				// set match results in context
				for (UInt sub = 0 ; sub < results.size() ; ++sub) {
					String name  = sub == 0 ? _("input") : String(_("_")) << sub;
					ctx.setVariable(name, to_script(results.str(sub)));
				}
				// call
				inside = replacement_function->eval(ctx)->toString();
			} else {
				inside = results.format(replacement_string);
			}
			// append replaced inside
			if (recursive && level < 20) {
				ret += apply(ctx, inside, level + 1);
			} else {
				ret += inside;
			}
			start = pos.second;
		}
		ret.append(start, input.end());
		return ret;
	}
};

SCRIPT_FUNCTION_WITH_SIMPLIFY(replace) {
	// construct replacer
	RegexReplacer replacer;
	replacer.match = from_script<ScriptRegexP>(ctx.getVariable(SCRIPT_VAR_match), SCRIPT_VAR_match);
	if (ctx.getVariableOpt(SCRIPT_VAR_in_context)) {
		replacer.context = from_script<ScriptRegexP>(ctx.getVariableOpt(SCRIPT_VAR_in_context), SCRIPT_VAR_in_context);
	}
	if (ctx.getVariableOpt(SCRIPT_VAR_recursive)) {
		replacer.recursive = from_script<bool>(ctx.getVariableOpt(SCRIPT_VAR_recursive), SCRIPT_VAR_recursive);
	} else {
		replacer.recursive = false;
	}
	replacer.replacement_function = ctx.getVariable(SCRIPT_VAR_replace);
	if (replacer.replacement_function->type() != SCRIPT_FUNCTION) {
		replacer.replacement_string = replacer.replacement_function->toString();
		replacer.replacement_function = ScriptValueP();
	}
	// run
	SCRIPT_PARAM_C(String, input);
	if (replacer.context || replacer.replacement_function || replacer.recursive) {
		SCRIPT_RETURN(replacer.apply(ctx, input));
	} else {
		// simple replacing
		replacer.match->replace_all(&input, replacer.replacement_string);
		SCRIPT_RETURN(input);
	}
}
SCRIPT_FUNCTION_SIMPLIFY_CLOSURE(replace) {
	FOR_EACH(b, closure.bindings) {
		if (b.first == SCRIPT_VAR_match || b.first == SCRIPT_VAR_in_context) {
			b.second = regex_from_script(b.second); // pre-compile
		}
	}
	return ScriptValueP();
}

// ----------------------------------------------------------------------------- : Rules : regex filter

SCRIPT_FUNCTION_WITH_SIMPLIFY(filter_text) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_PARAM_C(ScriptRegexP, match);
	SCRIPT_OPTIONAL_PARAM_C_(ScriptRegexP, in_context);
	String ret;
	// find all matches
	String::const_iterator start = input.begin();
	ScriptRegex::Results results;
	while (match->matches(results, input, start, in_context)) {
		// match, append to result
		ScriptRegex::Results::const_reference pos = results[0];
		ret.append(pos.first, pos.second);  // the match
		start = pos.second;
	}
	SCRIPT_RETURN(ret);
}
SCRIPT_FUNCTION_SIMPLIFY_CLOSURE(filter_text) {
	FOR_EACH(b, closure.bindings) {
		if (b.first == SCRIPT_VAR_match || b.first == SCRIPT_VAR_in_context) {
			b.second = regex_from_script(b.second); // pre-compile
		}
	}
	return ScriptValueP();
}

// ----------------------------------------------------------------------------- : Rules : regex break

SCRIPT_FUNCTION_WITH_SIMPLIFY(break_text) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_PARAM_C(ScriptRegexP, match);
	SCRIPT_OPTIONAL_PARAM_C_(ScriptRegexP, in_context);
	ScriptCustomCollectionP ret(new ScriptCustomCollection);
	// find all matches
	String::const_iterator start = input.begin();
	ScriptRegex::Results results;
	while (match->matches(results, input, start, in_context)) {
		// match, append to result
		ret->value.push_back(to_script(results.str()));
		start = results[0].second;
	}
	return ret;
}
SCRIPT_FUNCTION_SIMPLIFY_CLOSURE(break_text) {
	FOR_EACH(b, closure.bindings) {
		if (b.first == SCRIPT_VAR_match || b.first == SCRIPT_VAR_in_context) {
			b.second = regex_from_script(b.second); // pre-compile
		}
	}
	return ScriptValueP();
}

// ----------------------------------------------------------------------------- : Rules : regex split

SCRIPT_FUNCTION_WITH_SIMPLIFY(split_text) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_PARAM_C(ScriptRegexP, match);
	SCRIPT_PARAM_DEFAULT_N(bool, _("include empty"), include_empty, true);
	ScriptCustomCollectionP ret(new ScriptCustomCollection);
	// find all matches
	String::const_iterator start = input.begin();
	ScriptRegex::Results results;
	while (match->matches(results, start, input.end())) {
		// match, append the part before it to the result
		ScriptRegex::Results::const_reference pos = results[0];
		if (include_empty || pos.first != start) {
			ret->value.push_back(to_script( String(start,pos.first) ));
		}
		start = pos.second;
	}
	if (include_empty || start != input.end()) {
		ret->value.push_back(to_script( String(start,input.end()) ));
	}
	return ret;
}
SCRIPT_FUNCTION_SIMPLIFY_CLOSURE(split_text) {
	FOR_EACH(b, closure.bindings) {
		if (b.first == SCRIPT_VAR_match) {
			b.second = regex_from_script(b.second); // pre-compile
		}
	}
	return ScriptValueP();
}

// ----------------------------------------------------------------------------- : Rules : regex match

SCRIPT_FUNCTION_WITH_SIMPLIFY(match) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_PARAM_C(ScriptRegexP, match);
	SCRIPT_RETURN(match->matches(input));
}
SCRIPT_FUNCTION_SIMPLIFY_CLOSURE(match) {
	FOR_EACH(b, closure.bindings) {
		if (b.first == SCRIPT_VAR_match) {
			b.second = regex_from_script(b.second); // pre-compile
		}
	}
	return ScriptValueP();
}

// ----------------------------------------------------------------------------- : Init

void init_script_regex_functions(Context& ctx) {
	ctx.setVariable(_("replace"),              script_replace);
	ctx.setVariable(_("filter text"),          script_filter_text);
	ctx.setVariable(_("break text"),           script_break_text);
	ctx.setVariable(_("split text"),           script_split_text);
	ctx.setVariable(_("match"),                script_match);
	ctx.setVariable(_("replace rule"),         intrusive(new ScriptRule(script_replace)));
	ctx.setVariable(_("filter rule"),          intrusive(new ScriptRule(script_filter_text)));
	ctx.setVariable(_("break rule"),           intrusive(new ScriptRule(script_break_text)));
	ctx.setVariable(_("match rule"),           intrusive(new ScriptRule(script_match)));
}
