//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/word_list.hpp>

// ----------------------------------------------------------------------------- : WordList

WordListWord::WordListWord()
  : line_below(false)
  , is_prefix(false)
{}

IMPLEMENT_REFLECTION_NO_SCRIPT(WordListWord) {
  REFLECT_IF_READING_SINGLE_VALUE {
    // complex value
    REFLECT(name);
    REFLECT(line_below);
    REFLECT(is_prefix);
    REFLECT(words);
    REFLECT(script);
  } else {
    REFLECT_NAMELESS(name);
  }
}

IMPLEMENT_REFLECTION_NO_SCRIPT(WordList) {
  REFLECT(name);
  REFLECT(words);
}


// ----------------------------------------------------------------------------- : Auto replace words

AutoReplace::AutoReplace()
  : enabled(true)
  , whole_word(true)
  , custom(true)
{}

IMPLEMENT_REFLECTION_NO_SCRIPT(AutoReplace) {
  REFLECT(enabled);
  REFLECT(whole_word);
  REFLECT(match);
  REFLECT(replace);
}
