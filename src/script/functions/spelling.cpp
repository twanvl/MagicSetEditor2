
//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/spell_checker.hpp>
#include <util/tagged_string.hpp>
#include <data/stylesheet.hpp>

// ----------------------------------------------------------------------------- : Functions

inline size_t spelled_correctly(const String& input, size_t start, size_t end, SpellChecker** checkers, const ScriptValueP& extra_test, Context& ctx) {
  // untag
  String word = untag(input.substr(start,end-start));
  if (word.empty()) return true;
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
    if (extra_test->eval(ctx)->toBool()) {
      return true;
    }
    // try on tagged
    ctx.setVariable(SCRIPT_VAR_input, to_script(input.substr(start,end-start)));
    if (extra_test->eval(ctx)->toBool()) {
      return true;
    }
  }
  return false;
}

void check_word(const String& tag, const String& input, size_t start, size_t end, String& out, bool check, SpellChecker** checkers, const ScriptValueP& extra_test, Context& ctx) {
  if (start >= end) return;
  bool good = !check || spelled_correctly(input, start, end, checkers, extra_test, ctx);
  if (!good) { out += _("<"); out += tag; }
  out.append(input, start, end-start);
  if (!good) { out += _("</"); out += tag; }
}

SCRIPT_FUNCTION(check_spelling) {
  SCRIPT_PARAM_C(StyleSheetP,stylesheet);
  SCRIPT_PARAM_C(String,language);
  SCRIPT_PARAM_C(String,input);
  assert_tagged(input);
  if (!settings.stylesheetSettingsFor(*stylesheet).card_spellcheck_enabled)
    SCRIPT_RETURN(input);
  SCRIPT_OPTIONAL_PARAM_(String, extra_dictionary);
  SCRIPT_OPTIONAL_PARAM_(ScriptValueP, extra_match);
  // remove old spelling error tags
  input = remove_tag(input, _("<error-spelling"));
  // no language -> spelling checking
  if (language.empty()) {
    SCRIPT_RETURN(input);
  }
  SpellChecker* checkers[3] = {nullptr};
  checkers[0] = SpellChecker::get(language);
  if (!extra_dictionary.empty()) {
    checkers[1] = SpellChecker::get(extra_dictionary,language);
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
  size_t word_start = String::npos; // start of the word to be checked, or npos if not inside a word
  size_t pos = 0;
  int unchecked_tag = 0;
  while (pos < input.size()) {
    Char c = input.GetChar(pos);
    if (c == _('<')) {
      if      (is_tag(input, pos,  _("<nospellcheck"))) unchecked_tag++;
      else if (is_tag(input, pos, _("</nospellcheck"))) unchecked_tag--;
      else if (is_tag(input, pos,  _("<sym")))  unchecked_tag++;
      else if (is_tag(input, pos, _("</sym")))  unchecked_tag--;
      else if (is_tag(input, pos,  _("<atom"))) unchecked_tag++;
      else if (is_tag(input, pos, _("</atom"))) unchecked_tag--;
      // skip tag
      auto after = skip_tag(input,pos);
      if (word_start == pos) {
        // prefer to place word start inside tags, i.e. as late as possible
        word_start = pos = after;
        result.append(input, pos, after-pos);
      } else {
        if (word_start == String::npos) {
          result.append(input, pos, after - pos);
        }
        pos = after;
      }
    } else if (isAlpha(c)) {
      // a word character
      if (word_start == String::npos) word_start = pos;
      ++pos;
    } else {
      // a non-word character, punctuation or space
      // check word, add to result
      check_word(tag, input, word_start, pos, result, unchecked_tag <= 0, checkers, extra_match, ctx);
      word_start = String::npos;
      result += c;
      ++pos;
    }
  }
  // last word
  check_word(tag, input, word_start, input.size(), result, unchecked_tag <= 0, checkers, extra_match, ctx);
  // done
  assert_tagged(result);
  SCRIPT_RETURN(result);
}

SCRIPT_FUNCTION(check_spelling_word) {
  SCRIPT_PARAM_C(String,language);
  SCRIPT_PARAM_C(String,input);
  if (language.empty()) {
    // no language -> spelling checking
    SCRIPT_RETURN(true);
  } else {
    auto checker = SpellChecker::get(language);
    bool correct = !checker || checker->spell(input);
    SCRIPT_RETURN(correct);
  }
}

// ----------------------------------------------------------------------------- : Init

void init_script_spelling_functions(Context& ctx) {
  ctx.setVariable(_("check_spelling"),       script_check_spelling);
  ctx.setVariable(_("check_spelling_word"),  script_check_spelling_word);
}
