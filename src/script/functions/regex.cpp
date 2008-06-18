
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

class ScriptReplaceRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("replace_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM_C(String, input);
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
	SCRIPT_PARAM_C(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	// replace
	SCRIPT_PARAM_C(ScriptValueP, replace);
	if (replace->type() == SCRIPT_FUNCTION) {
		ret->replacement_function = replace;
	} else {
		ret->replacement = replace->toString();
	}
	// in_context
	SCRIPT_OPTIONAL_PARAM_C(String, in_context) {
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
		SCRIPT_PARAM_C(String, input);
		String ret;
		while (regex.Matches(input)) {
			// match, append to result
			size_t start, len;
			bool ok = regex.GetMatch(&start, &len, 0);
			assert(ok);
			String inside     = input.substr(start, len);  // the match
			String next_input = input.substr(start + len); // everything after the match
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
	// cached?
	SCRIPT_PARAM_C(String, match);
	SCRIPT_PARAM_DEFAULT_C(String, in_context, String());
	
	// cache
	/*
	const int CACHE_SIZE = 6;
	struct CacheItem{
		String match, in_context;
		intrusive_ptr<ScriptFilterRule> rule;
	};
	static CacheItem cache[CACHE_SIZE];
	static int cache_pos = 0;
	// find in cache?
	for (int i = 0 ; i < CACHE_SIZE ; ++i) {
		if (cache[i].rule && cache[i].match == match && cache[i].in_context == in_context) {
			return cache[i].rule;
		}
	}
	// add item
	cache[cache_pos].match = match;
	cache[cache_pos].in_context = in_context;
	cache[cache_pos].rule = intrusive_ptr<ScriptFilterRule>(new ScriptFilterRule);
	intrusive_ptr<ScriptFilterRule>& ret = cache[cache_pos].rule;
	cache_pos = (cache_pos+1) % CACHE_SIZE;
	/*/
	intrusive_ptr<ScriptFilterRule> ret(new ScriptFilterRule); 
	//*/
	
	// match
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	// in_context
	if (!in_context.empty()) {
		if (!ret->context.Compile(in_context, wxRE_ADVANCED)) {
			throw ScriptError(_("Error while compiling regular expression: '")+in_context+_("'"));
		}
	}
	return ret;
}

SCRIPT_FUNCTION(filter_rule) {
	return filter_rule(ctx);
}
SCRIPT_FUNCTION(filter_text) {
	return filter_rule(ctx)->eval(ctx);
}

// ----------------------------------------------------------------------------- : Rules : regex break

/*
class ScriptBreakRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("break_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM_C(String, input);
		intrusive_ptr<ScriptCustomCollection> ret(new ScriptCustomCollection);
		while (regex.Matches(input)) {
			// match, append to result
			size_t start, len;
			bool ok = regex.GetMatch(&start, &len, 0);
			assert(ok);
			String inside     = input.substr(start, len);  // the match
			String next_input = input.substr(start + len); // everything after the match
			if (!context.IsValid() || context.Matches(input.substr(0,start) + _("<match>") + next_input)) {
				// no context or context match
				ret->value.push_back(to_script(inside));
			}
			input = next_input;
		}
		return ret;
	}
	
	wxRegEx regex;   ///< Regex to match
	wxRegEx context; ///< Match only in a given context, optional
};

// Create a regular expression rule for breaking strings
ScriptValueP break_rule(Context& ctx) {
	intrusive_ptr<ScriptBreakRule> ret(new ScriptBreakRule);
	// match
	SCRIPT_PARAM_C(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	// in_context
	SCRIPT_OPTIONAL_PARAM_C(String, in_context) {
		if (!ret->context.Compile(in_context, wxRE_ADVANCED)) {
			throw ScriptError(_("Error while compiling regular expression: '")+in_context+_("'"));
		}
	}
	return ret;
}

SCRIPT_FUNCTION(break_rule) {
	return break_rule(ctx);
}
SCRIPT_FUNCTION(break_text) {
	return break_rule(ctx)->eval(ctx);
}*/

SCRIPT_FUNCTION_WITH_SIMPLIFY(break_text) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_PARAM_C(ScriptRegexP, match);
	SCRIPT_OPTIONAL_PARAM_C_(ScriptRegexP, in_context);
	intrusive_ptr<ScriptCustomCollection> ret(new ScriptCustomCollection);
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

// ----------------------------------------------------------------------------- : Rules : regex match


/*
class ScriptMatchRule : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("match_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		SCRIPT_PARAM_C(String, input);
		SCRIPT_RETURN(regex.Matches(input));
	}
	
	wxRegEx regex; ///< Regex to match
};

// Create a regular expression rule for filtering strings
ScriptValueP match_rule(Context& ctx) {
	intrusive_ptr<ScriptMatchRule> ret(new ScriptMatchRule);
	// match
	SCRIPT_PARAM_C(String, match);
	if (!ret->regex.Compile(match, wxRE_ADVANCED)) {
		throw ScriptError(_("Error while compiling regular expression: '")+match+_("'"));
	}
	return ret;
}

SCRIPT_FUNCTION(match_rule) {
	return match_rule(ctx);
}
SCRIPT_FUNCTION_WITH_SIMPLIFY(match) {
	return match_rule(ctx)->eval(ctx);
}
SCRIPT_FUNCTION_SIMPLIFY_CLOSURE(match) {
	ScriptValueP match = closure.getBinding(SCRIPT_VAR_match);
	if (match) {
		intrusive_ptr<ScriptMatchRule> ret(new ScriptMatchRule);
		from_script(match, ret->regex);
		return ret;
	}
	return ScriptValueP();
}
*/
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
	ctx.setVariable(_("match"),                script_match);
	ctx.setVariable(_("replace rule"),         script_replace_rule);
	ctx.setVariable(_("filter rule"),          script_filter_rule);
	ctx.setVariable(_("break rule"),           new_intrusive1<ScriptRule>(script_break_text));
	ctx.setVariable(_("match rule"),           new_intrusive1<ScriptRule>(script_match));
}
