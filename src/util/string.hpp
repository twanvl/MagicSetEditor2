//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file util/string.hpp
 *
 *  @brief String and character utility functions and macros
 */

// ----------------------------------------------------------------------------- : Includes

#include "prec.hpp"
#include "for_each.hpp"
#include <ctype.h>
#include <boost/preprocessor/cat.hpp>

class wxTextOutputStream;

// ----------------------------------------------------------------------------- : String type

/// The string type used throughout MSE
typedef wxString String;

inline wxStdString const& toStdString(String const& s) {
  #if wxUSE_UNICODE_WCHAR
    return s.ToStdWstring();
  #else
    return s.ToStdString();
  #endif
}

#if wxVERSION_NUMBER < 3100
// wxWidgets 3.1.0 added this specialization
namespace std {
  template<> struct hash<String> {
    size_t operator()(String const& s) const {
      return std::hash<std::wstring>()(s.ToStdWstring());
    }
  };
}
#endif

// ----------------------------------------------------------------------------- : Unicode

/// u if UNICODE is defined, a otherwise
#ifdef UNICODE
#  define IF_UNICODE(u,a) u
#else
#  define IF_UNICODE(u,a) a
#endif

#undef _
/// A string/character constant, correctly handled in unicode builds
#define _(S) IF_UNICODE(BOOST_PP_CAT(L,S), S)

/// The character type used
typedef wxChar Char;

/// UTF-8 Byte order mark for writing at the start of files
/** In non-unicode builds it is UTF8 encoded \xFEFF.
 *  In unicode builds it is a normal \xFEFF.
 */
const wchar_t BYTE_ORDER_MARK[] = L"\xFEFF";

/// Writes a string to an output stream, encoded as UTF8
void writeUTF8(wxTextOutputStream& stream, const String& str);

/// Remove a UTF-8 Byte order mark from an input stream
bool eat_utf8_bom(wxInputStream& input);

/// Some constants we like to use
#ifdef UNICODE
  #define  LEFT_ANGLE_BRACKET _("\x2039")
  #define RIGHT_ANGLE_BRACKET _("\x203A")
  #define  LEFT_SINGLE_QUOTE  _('\x2018')
  #define RIGHT_SINGLE_QUOTE  _('\x2019')
  #define  LEFT_DOUBLE_QUOTE  _('\x201C')
  #define RIGHT_DOUBLE_QUOTE  _('\x201D')
  #define EN_DASH             _('\x2013')
  #define EM_DASH             _('\x2014')
  #define CONNECTION_SPACE    _('\xEB00') // in private use area, untags to ' '
#else
  #define  LEFT_ANGLE_BRACKET _("<")
  #define RIGHT_ANGLE_BRACKET _(">")
  #define  LEFT_SINGLE_QUOTE  _('\'')
  #define RIGHT_SINGLE_QUOTE  _('\'')
  #define  LEFT_DOUBLE_QUOTE  _('\"')
  #define RIGHT_DOUBLE_QUOTE  _('\"')
  #define EN_DASH             _('-') // 150?
  #define EM_DASH             _('-') // 151?
  #define CONNECTION_SPACE    _(' ') // too bad
#endif

// ----------------------------------------------------------------------------- : Char functions

// Character set tests
inline bool isAlpha(Char c) { return IF_UNICODE( iswalpha(c) , isalpha((unsigned char)c) ); }
inline bool isDigit(Char c) { return IF_UNICODE( iswdigit(c) , isdigit((unsigned char)c) ); }
inline bool isAlnum(Char c) { return IF_UNICODE( iswalnum(c) , isalnum((unsigned char)c) ); }
inline bool isUpper(Char c) { return IF_UNICODE( iswupper(c) , isupper((unsigned char)c) ); }
inline bool isLower(Char c) { return IF_UNICODE( iswlower(c) , islower((unsigned char)c) ); }
inline bool isPunct(Char c) { return IF_UNICODE( iswpunct(c) , ispunct((unsigned char)c) ); }
// Character conversions
#ifdef _MSC_VER
  #define CHAR_FUNCTIONS_ARE_SLOW
#endif
#ifdef CHAR_FUNCTIONS_ARE_SLOW
  // These functions are slow as hell on msvc.
  // If also in other compilers, they can also use these routines.
  Char toLower(Char c);
  Char toUpper(Char c);
  inline bool isSpace(Char c) {
    if (c <= 128) {
      return (c >= 0x09 && c <= 0x0D) || c == 0x20;
    } else {
      return IF_UNICODE( iswspace(c) , isspace((unsigned char)c) ) || c == CONNECTION_SPACE;
    }
  }
#else
  inline Char toLower(Char c) { return IF_UNICODE( towlower(c) , tolower(c) ); }
  inline Char toUpper(Char c) { return IF_UNICODE( towupper(c) , toupper(c) ); }
  inline bool isSpace(Char c) { return IF_UNICODE( iswspace(c) , isspace((unsigned char)c) ) || c == CONNECTION_SPACE; }
#endif

// ----------------------------------------------------------------------------- : String view

// A view of (part of a string)
class StringView {
public:
  StringView(String const& str)
    : begin_(str.begin()), end_(str.end())
  {}
  StringView(String const& str, size_t pos)
    : begin_(str.begin() + pos), end_(str.end())
  {}
  StringView(String const& str, size_t pos, size_t count)
    : begin_(str.begin() + pos), end_(str.begin() + pos + min(count, str.size()-pos))
  {}
  StringView(String::const_iterator begin, String::const_iterator end)
    : begin_(begin), end_(end)
  {
    assert(begin <= end);
  }
  inline operator String () const {
    return String(begin_, end_);
  }
  using iterator = String::const_iterator;
  using const_iterator = String::const_iterator;
  inline String::const_iterator begin() const {
    return begin_;
  }
  inline String::const_iterator end() const {
    return end_;
  }
  inline size_t size() const {
    return end_ - begin_;
  }
  inline bool empty() const {
    return begin() == end();
  }
  inline bool operator == (StringView const& str) {
    return str.size() == size() && std::equal(begin(), end(), str.begin());
  }
  template <typename AnyChar>
  inline bool operator == (const AnyChar* str) {
    String::const_iterator it = begin_;
    while (true) {
      if (it == end_) return *str == '\0';
      if (*str == '\0') return false;
      if (*str != *it) return false;
      ++it; ++str;
    }
  }
private:
  String::const_iterator begin_, end_;
};

inline String& operator += (String& a, StringView b) {
  return a.append(b.begin(), b.end());
}

inline StringView substr(String const& str, size_t pos, size_t len) {
  return StringView(str, pos, len);
}
inline StringView substr(String const& str, size_t pos) {
  return StringView(str, pos);
}

// ----------------------------------------------------------------------------- : String utilities

/// Remove whitespace from both ends of a string
StringView trim(StringView);

/// Remove whitespace from the start of a string
StringView trim_left(StringView);

/// Replace the substring [start...end) of 'input' with 'replacement'
String substr_replace(const String& input, size_t start, size_t end, const String& replacement);

/// Replace all occurences of one needle with replacement
String replace_all(const String& heystack, const String& needle, const String& replacement);

/// Reverses a string, Note: std::reverse doesn't work with wxString
String reverse_string(String const& input);

// ----------------------------------------------------------------------------- : Caseing

/// Make each word in a string start with an upper case character.
/** for use in menus */
void capitalize_in_place(String&);
inline String capitalize(String const& s) {
  String result = s;
  capitalize_in_place(result);
  return result;
}

/// Make the first word in a string start with an upper case character.
/** for use in dialogs */
void capitalize_sentence_in_place(String&);
inline String capitalize_sentence(String const& s) {
  String result = s;
  capitalize_sentence_in_place(result);
  return result;
}

/// Convert a field name to canonical form
/** - converts ' ' to '_'
 */
void canonical_name_form_in_place(String&);
inline String canonical_name_form(String s) {
  canonical_name_form_in_place(s);
  return s;
}

/// Undo canonical_name_form: replace '_' by ' '
void uncanonical_name_form_in_place(String&);
inline String uncanonical_name_form(String s) {
  uncanonical_name_form_in_place(s);
  return s;
}

/// Convert a field name to a string that can be shown to the user
String name_to_caption(const String&);

/// Returns the singular form of a string
/** Used for reflection, for example "vector<T> apples" is written with keys
 *  singular_form("apples"), which is "apple"
 */
String singular_form(const String&);

// ----------------------------------------------------------------------------- : Comparing / finding

/// Compare two strings
/** Uses a smart comparison algorithm that understands numbers. 
 *  The comparison is case insensitive.
 *  Doesn't handle leading zeros.
 *
 *  Returns -1 if a < b, 0 if they are equal, and 1 if a > b
 */
int smart_compare(const String&, const String&);
/// Compare two strings, is the first less than the first?
bool smart_less(const String&, const String&);
/// Compare two strings for equality
bool smart_equal(const String&, const String&);

/// Return whether str starts with start
/** starts_with(a,b) == is_substr(a,0,b) */
//bool starts_with(const String& str, const String& start);
inline bool starts_with(StringView str, StringView const& start) {
  return str.size() >= start.size() && std::equal(start.begin(), start.end(), str.begin());
}
template <typename AnyChar>
inline bool starts_with(StringView str, const AnyChar* start) {
  String::const_iterator it = str.begin();
  while (true) {
    if (*start == '\0') return true;
    if (it == str.end()) return false;
    if (*start != *it) return false;
    ++it; ++start;
  }
}

/// Return whether str contains the string cmp at position pos
template <typename Cmp>
inline bool is_substr(const String& str, size_t pos, const Cmp& cmp) {
  return starts_with(StringView(str, pos), cmp);
}
/// Return whether begin..end contains the string cmp at position begin
template <typename It, typename Cmp>
inline bool is_substr(It begin, It end, const Cmp& cmp) {
  return starts_with(StringView(begin, end),cmp);
}

/// Return whether str contains the string cmp at position pos, case insensitive compare
bool is_substr_i(const String& str, size_t pos, const Char* cmp);
/// Return whether str contains the string cmp at position pos, case insensitive compare
bool is_substr_i(const String& str, size_t pos, const String& cmp);

/// Case insensitive string search, returns String::npos if not found
size_t find_i(const String& heystack, const String& needle);

/// Compare two strings for equality, a may contain '_' where b contains ' '
/** canoncial_name_compare(a,b) == (cannocial_name_form(a) == b)
 *  b should already be in cannonical name form
 */
bool canonical_name_compare(StringView a, const Char* b);

// ----------------------------------------------------------------------------- : Regular expressions

/// Escape a single character for use in regular expressions
String regex_escape(Char c);
/// Escape a string for use in regular expressions
String regex_escape(const String& s);

/// Make sure the given regex does no capturing
/** Basicly replaces "(" with "(?:" */
String make_non_capturing(const String& re);

// ----------------------------------------------------------------------------- : Iterator utilities

struct end_sentinel_t {};
const end_sentinel_t end_sentinel;

// Iterate over a string, removing all matching substrings.
// match.operator(it,end) should return false or return true and advance it past the substring
template <typename It, typename End, typename Match>
struct SkipSubstringIterator {
public:
  SkipSubstringIterator(It it, End end, Match const& match) : it(it), end(end), match(match) {
    while (match(it, end));
  }
  bool operator == (end_sentinel_t) const {
    return it == end;
  }
  bool operator != (end_sentinel_t) const {
    return it != end;
  }
  auto operator * () const {
    return *it;
  }
  auto& operator ++ () {
    ++it;
    while (match(it, end));
    return *this;
  }
private:
  It it;
  End end;
  Match match;
};

template <typename It, typename End, typename Match>
inline SkipSubstringIterator<It,End,Match> skip_substring_iterator(It it, End end, Match const& match) {
  return SkipSubstringIterator<It,End,Match>(it, end, match);
}
