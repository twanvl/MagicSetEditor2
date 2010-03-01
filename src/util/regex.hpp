//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_REGEX
#define HEADER_UTIL_REGEX

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
	/// Our own regular expression wrapper
	/** Suppors both boost::regex and wxRegEx.
	 *  Has an interface like boost::regex, but compatible with wxStrings.
	 */
	class Regex {
	  public:
		struct Results : public boost::match_results<const Char*> {
			/// Get a sub match
			inline String str(int sub = 0) const {
				const_reference v = (*this)[sub];
				return String(v.first, v.second);
			}
			/// Format a replacement string
			inline String format(const String& format) const {
				std::basic_string<Char> fmt(format.begin(),format.end());
				String output;
				boost::match_results<const Char*>::format(
					insert_iterator<String>(output, output.end()), fmt, boost::format_sed);
				return output;
			}
		};
		
		inline Regex() {}
		inline Regex(const String& code) { assign(code); }
		
		void assign(const String& code);
		inline bool matches(const String& str) const {
			return regex_search(str.begin(), str.end(), regex);
		}
		inline bool matches(Results& results, const String& str) const {
			return regex_search(str.begin(), str.end(), results, regex);
		}
		inline bool matches(Results& results, const Char* begin, const Char* end) const {
			return regex_search(begin, end, results, regex);
		}
		void replace_all(String* input, const String& format);
		
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

// ----------------------------------------------------------------------------- : EOF
#endif
