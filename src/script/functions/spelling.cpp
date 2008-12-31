
//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/spell_checker.hpp>
#include <util/tagged_string.hpp>

// ----------------------------------------------------------------------------- : Functions

inline size_t spelled_correctly(const String& input, size_t start, size_t end, SpellChecker** checkers, const ScriptValueP& extra_test, Context& ctx) {
	// untag
	String word = untag(input.substr(start,end-start));
	if (word.empty()) return true;
	// symbol?
	if (in_tag(input,_("<sym"),start,end) != String::npos) {
		// symbols are always spelled correctly
		return true;
	}
	// run through spellchecker(s)
	for (size_t i = 0 ; checkers[i] ; ++i) {
		if (checkers[i]->spell(word)) {
			return true;
		}
	}
	// run through additional words regex
	if (extra_test) {
		// try on untagged
		ctx.setVariable(SCRIPT_VAR_input, to_script(word));
		if (*extra_test->eval(ctx)) {
			return true;
		}
		// try on tagged
		ctx.setVariable(SCRIPT_VAR_input, to_script(input.substr(start,end-start)));
		if (*extra_test->eval(ctx)) {
			return true;
		}
	}
	return false;
}

void check_word(const String& tag, const String& input, String& out, size_t start, size_t end, SpellChecker** checkers, const ScriptValueP& extra_test, Context& ctx) {
	if (start >= end) return;
	bool good = spelled_correctly(input, start, end, checkers, extra_test, ctx);
	if (!good) out += _("<") + tag;
	out.append(input, start, end-start);
	if (!good) out += _("</") + tag;
}

void check_word(const String& tag, const String& input, String& out, Char sep, size_t prev, size_t start, size_t end, size_t after, SpellChecker** checkers, const ScriptValueP& extra_test, Context& ctx) {
	if (start == end) {
		// word consisting of whitespace/punctuation only
		if (untag(input.substr(prev,after-prev)).empty()) {
			if (isSpace(sep) && (after == input.size() || isSpace(input.GetChar(after)))) {
				// double space
				out += _("<error-spelling>");
				out.append(sep);
				out.append(input, prev, after-end);
				out += _("</error-spelling>");
			} else {
				if (sep) out.append(sep);
				out.append(input, prev, after-prev);
			}
		} else {
			// stand alone punctuation
			if (sep) out.append(sep);
			out += _("<error-spelling>");
			out.append(input, prev, after-end);
			out += _("</error-spelling>");
		}
	} else {
		// before the word
		if (sep) out.append(sep);
		out.append(input, prev, start-prev);
		// the word itself
		check_word(tag, input, out, start, end, checkers, extra_test, ctx);
		// after the word
		out.append(input, end, after-end);
	}
}

SCRIPT_FUNCTION(check_spelling) {
	SCRIPT_PARAM_C(String,language);
	SCRIPT_PARAM_C(String,input);
	SCRIPT_OPTIONAL_PARAM_N_(String,_("extra dictionary"),extra_dictionary);
	SCRIPT_OPTIONAL_PARAM_N_(ScriptValueP,_("extra match"),extra_match);
	// remove old spelling error tags
	input = remove_tag(input, _("<error-spelling"));
	// no language -> spelling checking
	if (language.empty()) {
		SCRIPT_RETURN(input);
	}
	SpellChecker* checkers[3] = {nullptr};
	checkers[0] = &SpellChecker::get(language);
	if (!extra_dictionary.empty()) {
		checkers[1] = &SpellChecker::get(extra_dictionary,language);
	}
	// what will the missspelling tag be?
	String tag = _("error-spelling:");
	tag += language;
	if (!extra_dictionary.empty()) {
		tag += _(":") + extra_dictionary;
	}
	tag += _(">");
	// now walk over the words in the input, and mark misspellings
	String result;
	Char sep = 0;
	size_t prev_end = 0, word_start = 0, word_end = 0, pos = 0;
	while (pos < input.size()) {
		Char c = input.GetChar(pos);
		if (c == _('<')) {
			if (word_start == pos) {
				// prefer to place word start inside tags, i.e. as late as possible
				word_end = word_start = pos = skip_tag(input,pos);
			} else {
				pos = skip_tag(input,pos);
			}
		} else if (isSpace(c) || c == EM_DASH || c == EN_DASH) {
			// word boundary => check the word
			check_word(tag, input, result, sep, prev_end, word_start, word_end, pos, checkers, extra_match, ctx);
			// next
			sep = c;
			prev_end = word_start = word_end = pos = pos + 1;
		} else {
			pos++;
			if (word_start == pos-1 && is_word_start_punctuation(c)) {
				// skip punctuation at start of word
				word_end = word_start = pos;
			} else if (is_word_end_punctuation(c)) {
				// skip punctuation at end of word
			} else {
				word_end = pos;
			}
		}
	}
	// last word
	check_word(tag, input, result, sep, prev_end, word_start, word_end, pos, checkers, extra_match, ctx);
	// done
	SCRIPT_RETURN(result);
}

SCRIPT_FUNCTION(check_spelling_word) {
	SCRIPT_PARAM_C(String,language);
	SCRIPT_PARAM_C(String,input);
	if (language.empty()) {
		// no language -> spelling checking
		SCRIPT_RETURN(true);
	} else {
		bool correct = SpellChecker::get(language).spell(input);
		SCRIPT_RETURN(correct);
	}
}

// ----------------------------------------------------------------------------- : Init

void init_script_spelling_functions(Context& ctx) {
	ctx.setVariable(_("check spelling"),       script_check_spelling);
	ctx.setVariable(_("check spelling word"),  script_check_spelling_word);
}
