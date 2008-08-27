//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/error.hpp>

DECLARE_POINTER_TYPE(ScriptRegex);
DECLARE_TYPEOF_COLLECTION(pair<Variable COMMA ScriptValueP>);

// ----------------------------------------------------------------------------- : Regex type

/// A regular expression for use in a script
class ScriptRegex : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_REGEX; }
	virtual String typeName() const { return _("regex"); }
	
	wxRegEx regex; ///< The regular expression
};

ScriptRegexP regex_from_script(const ScriptValueP& value) {
	// is it a regex already?
	ScriptRegexP regex = dynamic_pointer_cast<ScriptRegex>(value);
	if (!regex) {
		// TODO: introduce some kind of caching?
		// compile string
		regex = new_intrusive<ScriptRegex>();
		if (!regex->regex.Compile(*value, wxRE_ADVANCED)) {
			throw ScriptError(_("Error while compiling regular expression: '") + value->toString() + _("'"));
		}
		assert(regex->regex.IsValid());
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
	
	String apply(Context& ctx, String& input, int level = 0) const {
		// match first, then check context of match
		String ret;
		while (match->regex.Matches(input)) {
			// for each match ...
			size_t start, len;
			bool ok = match->regex.GetMatch(&start, &len, 0);
			assert(ok);
			ret                 += input.substr(0, start);          // everything before the match position stays
			String inside        = input.substr(start, len);        // inside the match
			String next_input    = input.substr(start + len);       // next loop the input is after this match
			if (!context || context->regex.Matches(ret + _("<match>") + next_input)) {
				// the context matches -> perform replacement
				if (replacement_function) {
					// set match results in context
					for (UInt m = 0 ; m < match->regex.GetMatchCount() ; ++m) {
						match->regex.GetMatch(&start, &len, m);
						String name  = m == 0 ? _("input") : String(_("_")) << m;
						String value = input.substr(start, len);
						ctx.setVariable(name, to_script(value));
					}
					// call
					inside = replacement_function->eval(ctx)->toString();
				} else {
					match->regex.Replace(&inside, replacement_string, 1); // replace inside
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
		replacer.match->regex.Replace(&input, replacer.replacement_string);
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
	while (match->regex.Matches(input)) {
		// match, append to result
		size_t start, len;
		bool ok = match->regex.GetMatch(&start, &len, 0);
		assert(ok);
		String inside     = input.substr(start, len);  // the match
		String next_input = input.substr(start + len); // everything after the match
		if (!in_context || in_context->regex.Matches(input.substr(0,start) + _("<match>") + next_input)) {
			// no context or context match
			ret += inside;
		}
		input = next_input;
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
	while (match->regex.Matches(input)) {
		// match, append to result
		size_t start, len;
		bool ok = match->regex.GetMatch(&start, &len, 0);
		assert(ok);
		String inside     = input.substr(start, len);  // the match
		String next_input = input.substr(start + len); // everything after the match
		if (!in_context || in_context->regex.Matches(input.substr(0,start) + _("<match>") + next_input)) {
			// no context or context match
			ret->value.push_back(to_script(inside));
		}
		input = next_input;
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
	while (match->regex.Matches(input)) {
		// match, append to result
		size_t start, len;
		bool ok = match->regex.GetMatch(&start, &len, 0);
		assert(ok);
		if (include_empty || start > 0) {
			ret->value.push_back(to_script(input.substr(0,start)));
		}
		input = input.substr(start + len); // everything after the match
	}
	if (include_empty || !input.empty()) {
		ret->value.push_back(to_script(input));
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
	SCRIPT_RETURN(match->regex.Matches(input));
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
	ctx.setVariable(_("replace rule"),         new_intrusive1<ScriptRule>(script_replace));
	ctx.setVariable(_("filter rule"),          new_intrusive1<ScriptRule>(script_filter_text));
	ctx.setVariable(_("break rule"),           new_intrusive1<ScriptRule>(script_break_text));
	ctx.setVariable(_("match rule"),           new_intrusive1<ScriptRule>(script_match));
}
