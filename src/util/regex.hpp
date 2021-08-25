//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// Use boost::regex as opposed to wxRegex
/* 2008-09-01:
 *     Script profiling shows that the boost library is significantly faster:
 *     When loading a large magic set (which calls ScriptManager::updateAll):
 *            function      Calls   wxRegEx    boost
 *            ------------------------------------------------------------------
 *            replace        3791   0.38607    0.20857
 *            filter_text      11   0.32251    0.02446
 *
 *            (times are avarage over all calls, in ms)
 */
#define USE_BOOST_REGEX 1

#if USE_BOOST_REGEX
  #include <boost/regex.hpp>
  #include <boost/regex/pattern_except.hpp>
#endif

// ----------------------------------------------------------------------------- : Boost implementation

#if USE_BOOST_REGEX
  // needed for boost::regex to compute hash values of unicode chars
  #if BOOST_VERSION < 107600
    inline std::size_t hash_value(wxUniChar const& x) {
      boost::hash<int> hasher;
      return hasher(static_cast<int>(x));
    }
  #else
    // boost > 1.76 uses its own hash function that needs operator +
    inline int operator + (wxUniChar x, unsigned int y) {
      return static_cast<int>(x) + y;
    }
  #endif

  /// Our own regular expression wrapper
  /** Suppors both boost::regex and wxRegEx.
   *  Has an interface like boost::regex, but compatible with wxStrings.
   */
  class Regex {
  public:
    struct Results : public boost::match_results<String::const_iterator> {
      /// Get a sub match
      inline String str(int sub = 0) const {
        const_reference v = (*this)[sub];
        return String(v.first, v.second);
      }
      /// Format a replacement string
      inline String format(const String& format) const {
        std::basic_string<Char> fmt(format.begin(),format.end());
        String output;
        boost::match_results<String::const_iterator>::format(
          insert_iterator<String>(output, output.end()), fmt, boost::format_sed);
        return output;
      }
    };
    
    inline Regex() {}
    inline Regex(const String& code) { assign(code); }
    
    void assign(const String& code);
    inline bool matches(const String& str) const {
      return regex_search(toStdString(str), regex);
    }
    inline bool matches(Results& results, const String& str, size_t start = 0) const {
      return matches(results, str.begin() + start, str.end());
    }
    inline bool matches(Results& results, const String::const_iterator& begin, const String::const_iterator& end) const {
      return regex_search(begin, end, results, regex);
    }
    String replace_all(const String& input, const String& format) const;
    
    inline bool empty() const {
      return regex.empty();
    }
    
  private:
    boost::basic_regex<Char> regex; ///< The regular expression
  };

// ----------------------------------------------------------------------------- : Wx implementation
#else
  /// Our own regular expression wrapper
  /** Suppors both boost::regex and wxRegEx.
   *  Has an interface like boost::regex.
   */
  class Regex 
  public:
    // Interface for compatability with boost::regex
    class Results {
    public:
      typedef pair<const Char*,const Char*> value_type; // (begin,end)
      typedef value_type const_reference;
      /// Number of submatches (+1 for the total match)
      inline size_t size() const { return regex->GetMatchCount(); }
      /// Get a submatch
      inline value_type operator [] (int sub) const {
        size_t pos, length;
        bool ok = regex->GetMatch(&pos, &length, sub);
        assert(ok);
        return make_pair(begin + pos, begin + pos + length);
      }
      inline size_t position(int sub = 0) const {
        size_t pos, length;
        bool ok = regex->GetMatch(&pos, &length, sub);
        assert(ok);
        return pos;
      }
      inline size_t length(int sub = 0) const {
        size_t pos, length;
        bool ok = regex->GetMatch(&pos, &length, sub);
        assert(ok);
        return length;
      }
      /// Get a sub match
      inline String str(int sub = 0) const {
        const_reference v = (*this)[sub];
        return String(v.first, v.second);
      }
      /// Format a replacement string
      inline String format(const String& format) const {
        const_reference v = (*this)[0];
        String inside(v.first, v.second);
        regex->ReplaceFirst(&inside, format);
        return inside;
      }
    private:
      wxRegEx*    regex;
      const Char* begin;
      friend class ScriptRegex;
    };
    
    inline Regex() {}
    inline Regex(const String& code) { assign(code); }
    
    void assign(const String& code);
    inline bool matches(const String& str) const {
      return regex.Matches(str);
    }
    inline bool matches(Results& results, const String& str) const {
      results.regex = &regex;
      results.begin = str.begin();
      return regex.Matches(str);
    }
    inline bool matches(Results& results, const Char* begin, const Char* end) const {
      results.regex = &regex;
      results.begin = begin;
      return regex.Matches(begin, 0, end - begin);
    }
    inline void replace_all(String* input, const String& format) {
      regex.Replace(input, format);
    }
    inline bool empty() const {
      return !regex.IsValid();
    }
    
  private:
    wxRegEx regex; ///< The regular expression
  };

#endif

