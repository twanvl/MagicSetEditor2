//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/spec_sort.hpp>
#include <util/error.hpp>

const Char REMOVED     = _('\0');
const Char PLACEHOLDER = _('\3');

String spec_sort(const String& spec, String& input, String& ret);

// ----------------------------------------------------------------------------- : Iterator for reading specs

/// Iterator over a sort specification (for spec_sort)
class SpecIterator {
  public:
  SpecIterator(const String& spec, size_t pos = 0)
    : spec(spec), pos(pos)
  {}
  
  wxUniChar value; ///< Current character
  bool escaped; ///< Was the current character escaped?
  bool preceded_by_space; ///< Was there a ' ' before this character?
  
  /// Move to the next item in the specification.
  /** returns false if we are at the end or encounter close.
   */
  bool nextUntil(wxUniChar close, bool skip_space = true) {
    if (pos >= spec.size()) {
      value = 0;
      if (close == 0) {
        return false;
      } else {
        throw ParseError(String::Format(_("Expected '%c' in sort_rule specification"),close));
      }
    }
    value = spec.GetChar(pos++);
    preceded_by_space = false;
    // skip whitespace
    if (skip_space) {
      while (value == _(' ')) {
        if (pos >= spec.size()) {
          if (close == 0) {
            return false;
          } else {
            throw ParseError(String::Format(_("Expected '%c' in sort_rule specification"),close));
          }
        }
        value = spec.GetChar(pos++);
        preceded_by_space = true;
      }
    }
    // escape?
    if (value == _('\\')) {
      escaped = true;
      if (pos >= spec.size()) {
        throw ParseError(String::Format(_("Expected '%c' in sort_rule specification"),close));
      }
      value = spec.GetChar(pos++);
    } else {
      escaped = false;
      if (value == close) return false;
    }
    return true;
  }
  
  /// Read a whole parameter, terminated by close
  String readParam(Char close, bool skip_space = true) {
    String ret;
    while(nextUntil(close)) ret += value;
    return ret;
  }
  
  /// Read a parameter, matches nested parentheses, keeps escape sequences
  String readRawParam(Char close1, Char close2 = 0) {
    String ret;
    int parens = 0;
    while (nextUntil(0, false)) {
      if (escaped) {
        ret += _('\\');
      } else {
        if (parens == 0 && (value == close1 || value == close2)) break;
        if      (value == _('(')) parens++;
        else if (value == _(')')) parens--;
      }
      ret += value;
    }
    return ret;
  }
  
  /// Does the current position match a keyword? If so, skip it
  bool keyword(const Char* kw) {
    if (value == kw[0]) {
      if (is_substr(spec, pos, kw + 1)) {
        pos += wxStrlen(kw + 1);
        return true;
      }
    }
    return false;
  }
  
  private:
  const String& spec;
  size_t pos;
};

// ----------------------------------------------------------------------------- : Sort functions

//using Bag = vector<wxUniChar>;
using Bag = String;

size_t count_and_remove(wxUniChar c, Bag& input) {
  size_t count = 0;
  size_t j=0;
  for (size_t i=0 ; i < input.size() ; ++i) {
    if (input[i] == c) {
      count++;
      input[i] = REMOVED;
    } else {
      //input[j++] = input[i];
    }
  }
  //input.resize(j);
  return count;
}

/// Sort a string using a specification using the shortest cycle method, see spec_sort
/** Removed used characters from input! */
void cycle_sort(const String& spec, Bag& input, Bag& ret) {
  // count occurences of each item in spec
  vector<size_t> counts;
  for(auto s : spec) {
    counts.push_back(count_and_remove(s, input));
  }
  // determine best start point
  size_t best_start = 0;
  size_t best_start_score = 0xffffffff;
  for (size_t start = 0 ; start < spec.size() ; ++start) {
    // score of a start position, can be considered as:
    //  - count saturated to binary
    //  - rotated left by start
    //  - interpreted as a binary number, but without trailing 0s
    size_t score = 0, mul = 1;
    for (size_t i = 0 ; i < spec.size() ; ++i) {
      mul *= 2;
      if (counts[(start + i) % spec.size()]) {
        score = score * mul + 1;
        mul = 1;
      }
    }
    if (score < best_start_score) {
      best_start_score = score;
      best_start       = start;
    }
  }
  // add to return string
  for (size_t i = 0 ; i < spec.size() ; ++i) {
    size_t pos = (best_start + i) % spec.size();
    ret.append(counts[pos], spec[pos]);
  }
}

/// Sort a string, keeping the characters in the original order
/** Removed used characters from input! */
void mixed_sort(const String& spec, Bag& input, Bag& ret) {
  size_t j = 0;
  for (wxUniCharRef c : input) {
    if (spec.find(c) != String::npos) {
      ret += c;
      c = REMOVED;
    }
  }
}

/// Sort a string, find a compound item
/** Removed used characters from input! */
void compound_sort(const String& spec, Bag& input, Bag& ret) {
  size_t j=0;
  for (size_t i=0 ; i < input.size() ; ++i) {
    // match?
    if (i+spec.size() <= input.size() && std::equal(spec.begin(), spec.end(), input.begin()+i)) {
      i += spec.size() - 1;
      ret += spec;
      input[i] = REMOVED;
    }
  }
}

/// Sort things matching a pattern
void pattern_sort(const String& pattern, const String& spec, Bag& input, Bag& ret) {
  if (pattern.size() > input.size()) return;
  size_t end = input.size() - pattern.size() + 1;
  size_t pos_new = 0;
  //for (size_t pos = 0 ; pos < end ; ++pos) {
  for (size_t pos = 0; pos < end; ++pos) {
    if (pos + pattern.size() > input.size()) {
      goto no_match;
    }
    {
      // does the pattern match here?
      String placeholders;
      for (size_t j = 0; j < pattern.size(); ++j) {
        wxUniChar c = input[pos + j];
        wxUniChar p = pattern[j];
        if (c == REMOVED) {
          goto no_match;
        } else if (p == PLACEHOLDER) {
          placeholders += c;
        } else if (c != p) {
          goto no_match;
        }
      }
      // we have a match
      // sort placeholders
      String new_placeholders = spec_sort(spec, placeholders);
      if (new_placeholders.size() == placeholders.size()) {
        // add to output, erase from input
        size_t ph = 0;
        for (size_t j = 0; j < pattern.size(); ++j) {
          wxUniChar p = pattern[j];
          if (p == PLACEHOLDER) {
            ret += new_placeholders[ph++];
          } else {
            ret += p;
          }
        }
        // skip over matched pattern
        pos += pattern.size() - 1;
        continue;
      }
    }
  no_match:
    input[pos_new++] = input[pos];
  }
  input.resize(pos_new);
}

/// Sort things in place, keep the rest of the input
void in_place_sort(const String& spec, String& input, String& ret) {
  String result;
  spec_sort(spec, input, result);
  // restore into the same order as in 'input'
  size_t pos_r = 0;
  FOR_EACH_CONST(c, input) {
    if (c == REMOVED) {
      if (pos_r < result.size()) {
        ret += result.GetChar(pos_r++);
      }
    } else {
      ret += c;
    }
  }
  input.clear(); // we ate all the input
}

// ----------------------------------------------------------------------------- : spec_sort

String spec_sort(const String& spec, String& input, String& ret) {
  SpecIterator it(spec);
  while(it.nextUntil(0)) {
    if (it.escaped) { // single character, escaped
      size_t count = count_and_remove(it.value, input);
      ret.append(count, it.value);
    } else if (it.value == _('<')) { // keep only a single copy
      while (it.nextUntil(_('>'))) {
        size_t pos = input.find_first_of(it.value);
        if (pos != String::npos) {
          input.erase(pos, 1);
          ret += it.value; // input contains it.value
        }
      }
    } else if (it.keyword(_("once("))) {
      while (it.nextUntil(_(')'))) {
        size_t pos = input.find_first_of(it.value);
        if (pos != String::npos) {
          input.erase(pos, 1);
          ret += it.value; // input contains it.value
        }
      }
      
    } else if (it.value == _('[')) {  // in input order
      mixed_sort(it.readParam(_(']')), input, ret);
    } else if (it.keyword(_("mixed("))) {
      mixed_sort(it.readParam(_(')')), input, ret);
      
    } else if (it.keyword(_("cycle("))) {
      cycle_sort(it.readParam(_(')')), input, ret);
    } else if (it.value == _('(')) {
      cycle_sort(it.readParam(_(')')), input, ret);
    
    } else if (it.keyword(_("compound("))) { // compound item
      compound_sort(it.readParam(_(')')), input, ret);
    
    } else if (it.keyword(_("pattern("))) { // recurse with pattern
      String pattern;
      // read pattern
      while (it.nextUntil(_(' '), false)) {
        if (it.value == _('.') && !it.escaped) {
          pattern += PLACEHOLDER;
        } else {
          pattern += it.value;
        }
      }
      // read spec to apply to pattern
      String sub_spec = it.readRawParam(_(')'));
      // sort
      pattern_sort(pattern, sub_spec, input, ret);
    
    } else if (it.keyword(_("in_place("))) { // recurse without pattern
      // read spec to apply to pattern
      String sub_spec = it.readRawParam(_(')'));
      in_place_sort(sub_spec, input, ret);
    
    } else if (it.keyword(_("any()"))) { // remaining input
      FOR_EACH_CONST(d, input) {
        if (d != REMOVED) {
          ret += d;
        }
      }
      input.clear();
    
    } else if (it.keyword(_("reverse_order("))) { // reverse order of preference
      vector<String> parts;
      while (it.value != _(')')) {
        String sub_spec = it.readRawParam(_(')'),_(' '));
        String part;
        spec_sort(sub_spec, input, part);
        parts.push_back(part);
      }
      // add parts in reverse order
      reverse(parts.begin(), parts.end());
      for (auto const& part : parts) {
        ret += part;
      }
    
    } else if (it.keyword(_("ordered("))) { // in spec order
      while (it.nextUntil(_(')'))) {
        size_t count = count_and_remove(it.value, input);
        ret.append(count, it.value);
      }
    } else { // single char
      size_t count = count_and_remove(it.value, input);
      ret.append(count, it.value);
    }
  }
  return ret;
}

String spec_sort(const String& spec, String input) {
  String ret;
  spec_sort(spec, input, ret);
  return ret;
}
