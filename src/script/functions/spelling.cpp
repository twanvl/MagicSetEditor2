
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

void check_word(const String& input, String& out, size_t start, size_t end, SpellChecker& checker, bool must_be_empty) {
	if (start >= end) return;
	String word = untag(input.substr(start,end-start));
	// TODO: handle keywords and cardname references
	bool error = !word.empty() && (must_be_empty || !checker.spell_with_punctuation(word));
	if (error) out += _("<error-spelling>");
	out.append(input, start, end-start);
	if (error) out += _("</error-spelling>");
}

SCRIPT_FUNCTION(check_spelling) {
	SCRIPT_PARAM(String,language);
	SCRIPT_PARAM(String,input);
	if (language.empty()) {
		// no language -> spelling checking
		SCRIPT_RETURN(true);
	}
	SpellChecker& checker = SpellChecker::get(language);
	// remove old spelling error tags
	input = remove_tag(input, _("<error-spelling"));
	// now walk over the words in the input, and mark misspellings
	String result;
	size_t word_start = 0, pos = 0;
	bool must_be_empty = false; // must this word be empty?
	while (pos < input.size()) {
		Char c = input.GetChar(pos);
		if (c == _('<')) {
			if (is_substr(input,pos,_("<sym"))) {
				// before symbols should be empty
				check_word(input,result, word_start,pos, checker, true);
				// don't spellcheck symbols
				word_start = pos;
				pos = min(input.size(), match_close_tag_end(input,pos));
				result.append(input, word_start, pos-word_start);
				word_start = pos;
				must_be_empty = true; // need a space after symbols
			} else {
				pos = skip_tag(input,pos);
			}
		} else if (isSpace(c)) {
			// word boundary -> check word
			check_word(input,result, word_start,pos, checker, must_be_empty);
			// next
			result += c;
			pos++;
			word_start = pos;
			must_be_empty = false;
		} else {
			pos++;
		}
	}
	// last word
	check_word(input,result, word_start,input.size(), checker, must_be_empty);
	// done
	SCRIPT_RETURN(result);
}

SCRIPT_FUNCTION(check_spelling_word) {
	SCRIPT_PARAM(String,language);
	SCRIPT_PARAM(String,input);
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
