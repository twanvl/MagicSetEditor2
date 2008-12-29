//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/string.hpp>
#include <util/for_each.hpp>
#include <wx/txtstrm.h>

// ----------------------------------------------------------------------------- : Unicode

String decodeUTF8BOM(const String& s) {
	#ifdef UNICODE
		if (!s.empty() && s.GetChar(0) == L'\xFEFF') {
			// skip byte-order-mark
			return s.substr(1);
		} else {
			return s;
		}
	#else
		wxWCharBuffer buf = s.wc_str(wxConvUTF8);
		if (buf && buf[size_t(0)] == L'\xFEFF') {
			// skip byte-order-mark
			return String(buf + 1, *wxConvCurrent);
		} else {
			return String(buf,     *wxConvCurrent);
		}
	#endif
}

void writeUTF8(wxTextOutputStream& stream, const String& str) {
	#ifdef UNICODE
		stream.WriteString(str);
	#else
		wxWCharBuffer buf = str.wc_str(*wxConvCurrent);
		stream.WriteString(wxString(buf, wxConvUTF8));
	#endif
}

// ----------------------------------------------------------------------------- : Char functions

#ifdef CHAR_FUNCTIONS_ARE_SLOW

Char toLower(Char c) {
	if (c <= 128) {
		if (c >= _('A') && c <= _('Z')) return c + (_('a') - _('A'));
		else                            return c;
	} else {
		return IF_UNICODE( towlower(c) , tolower(c) );
	}
}

Char toUpper(Char c) {
	if (c <= 128) {
		if (c >= _('a') && c <= _('z')) return c + (_('A') - _('a'));
		else                            return c;
	} else {
		return IF_UNICODE( towupper(c) , toupper(c) );
	}
}

#endif

// ----------------------------------------------------------------------------- : String utilities

String trim(const String& s){
	size_t start = s.find_first_not_of(_(" \t"));
	size_t end   = s.find_last_not_of( _(" \t"));
	if (start == String::npos) {
		return String();
	} else if (start == 0 && end == s.size() - 1) {
		return s;
	} else {
		return s.substr(start, end - start + 1);
	}
}

String trim_left(const String& s) {
	size_t start = s.find_first_not_of(_(" \t"));
	if (start == String::npos) {
		return String();
	} else {
		return s.substr(start);
	}
}

String substr_replace(const String& input, size_t start, size_t end, const String& replacement) {
	return input.substr(0,start) + replacement + input.substr(end);
}

String replace_all(const String& heystack, const String& needle, const String& replacement) {
	String ret = heystack;
	ret.Replace(needle, replacement);
	return ret;
}

// ----------------------------------------------------------------------------- : Words

String last_word(const String& s) {
	size_t endLastWord   = s.find_last_not_of(_(' '));
	size_t startLastWord = s.find_last_of(    _(' '), endLastWord);
	if (endLastWord == String::npos) {
		return String(); // empty string
	} else if (startLastWord == String::npos) {
		return s.substr(0, endLastWord + 1);// first word
	} else {
		return s.substr(startLastWord + 1, endLastWord - startLastWord);
	}
}

String strip_last_word(const String& s) {
	size_t endLastWord   = s.find_last_not_of(_(' '));
	size_t startLastWord = s.find_last_of(_(' '), endLastWord);
	if (endLastWord == String::npos || startLastWord == String::npos) {
		return String(); // single word or empty string
	} else {
		return s.substr(0, startLastWord + 1);
	}
}

const String word_start = String(_("[({\"\'")) + LEFT_SINGLE_QUOTE + LEFT_DOUBLE_QUOTE;
const String word_end   = String(_("])}.,;:?!\"\'")) + RIGHT_SINGLE_QUOTE + RIGHT_DOUBLE_QUOTE;

void trim_punctuation(const String& str, size_t& start, size_t& end) {
	start = str.find_first_not_of(word_start, start);
	end   = str.find_last_not_of(word_end,    min(end,str.size()-1)) + 1;
	if (start >= end) start = end;
}

// ----------------------------------------------------------------------------- : Caseing

/// Quick check to see if the substring starting at the given iterator is equal to some given string
bool is_substr(const String& s, String::iterator it, const Char* cmp) {
	while (it != s.end() && *cmp != 0) {
		if (*it++ != *cmp++) return false;
	}
	return *cmp == 0;
}

String capitalize(const String& s) {
	String result = s;
	bool after_space = true;
	FOR_EACH_IT(it, result) {
		if (*it == _(' ') || *it == _('/')) {
			after_space = true;
		} else if (after_space) {
			after_space = false;
			if (it != s.begin() &&
			    (is_substr(result,it,_("is ")) || is_substr(result,it,_("the ")) ||
			     is_substr(result,it,_("in ")) || is_substr(result,it,_("of "))  ||
			     is_substr(result,it,_("to ")) || is_substr(result,it,_("at "))  ||
			     is_substr(result,it,_("a " )))) {
				// Short words are not capitalized, keep lower case
			} else {
				*it = toUpper(*it);
			}
		}
	}
	return result;
}

String capitalize_sentence(const String& s) {
	String ret = s;//.Lower();
	if (!ret.empty()) {
		ret[0] = toUpper(ret[0]);
	}
	return ret;
}

String cannocial_name_form(const String& str) {
	String ret;
	ret.reserve(str.size());
	bool leading = true;
	FOR_EACH_CONST(c, str) {
		if ((c == _('_') || c == _(' '))) {
			ret += leading ? c : _(' ');
		} else {
			ret += c;
			leading = false;
/*			
		} else if (isAlnum(c) || c == _('-')) {
			ret += toLower(c);
			leading = false;
		} else {
			// ignore non alpha numeric*/
		}
	}
	return ret;
}

String singular_form(const String& str) {
	assert(str.size() > 1);
	assert(str.GetChar(str.size() - 1) == _('s')); // ends in 's'
	if (str.size() > 3 && is_substr(str, str.size()-3, _("ies"))) {
		return str.substr(0, str.size() - 3) + _("y");
	}
	return str.substr(0, str.size() - 1);
}

String remove_shortcut(const String& str) {
	size_t tab = str.find_last_of(_('\t'));
	if (tab == String::npos) return str;
	else                     return str.substr(0, tab);
}

// ----------------------------------------------------------------------------- : Comparing / finding

// Nice unicode normalization tables, probably not conform the standards
char latin_1[] = "aaaaaaaceeeeiiii"
                 "dnooooo ouuuuy  "
                 "aaaaaaaceeeeiiii"
                 "dnooooo ouuuuy y";
char latin_A[] = "aaaaaaccccccccdd"
                 "ddeeeeeeeeeegggg"
                 "gggghhhhiiiiiiii"
                 "iiiijjkkklllllll"
                 "lllnnnnnnnnnoooo"
                 "oooorrrrrrssssss"
                 "ssttttttuuuuuuuu"
                 "uuuuwwyyyzzzzzzs";
char latin_B[] = "bbbbbbcccdddddee"
                 "effgg iikkllmnno"
                 "oo  pprssssttttu"
                 "uuuyyzz         "
                 "    dddlllnnnaai"
                 "ioouuuuuuuuuueaa"
                 "aaaaggggkkoooo  "
                 "jdddgg  nnaaaaoo"
                 "aaaaeeeeiiiioooo"
                 "rrrruuuusstt  hh"
                 "nd  zzaaeeoooooo"
                 "ooyylntj  acclts"
                 "z  buveejjqqrryy";
char latin_E[] = "aabbbbbbccdddddd"
                 "ddddeeeeeeeeeeff"
                 "gghhhhhhhhhhiiii"
                 "kkkkkkllllllllmm"
                 "mmmmnnnnnnnnoooo"
                 "oooopppprrrrrrrr"
                 "sssssssssstttttt"
                 "ttuuuuuuuuuuvvvv"
                 "wwwwwwwwwwxxxxyy"
                 "zzzzzzhtwyas    "
                 "aaaaaaaaaaaaaaaa"
                 "aaaaaaaaeeeeeeee"
                 "eeeeeeeeiiiioooo"
                 "oooooooooooooooo"
                 "oooouuuuuuuuuuuu"
                 "uuyyyyyyyy      ";

/// Remove accents from a (lowercase) character
Char remove_accents(Char c) {
	char dec = ' ';
	if (c >= 0xC0) {
		if (c <= 0xFF) { // Latin 1
			dec = latin_1[c - 0xC0];
		} else if (c <= 0x17E) { // Latin extended A
			dec = latin_A[c - 0x100];
		} else if (c <= 0x180 && c <= 0x240) { // Latin extended B
			dec = latin_B[c - 0x180];
		} else if (c <= 0x1E00 && c <= 0x1EFF) { // Latin additional
			dec = latin_E[c - 0x1E00];
		}
	}
	return dec == ' ' ? toLower(c) : dec;
}

/// Is c a precomposed character (not counting accent marks)
/** If so, returns the second character of the decomposition */
Char decompose_char2(Char c) {
	if (c <  0xC6) {
		return 0;
	} else if (c == 0xC6 || c == 0xE6 || c == 0x152 || c == 0x153 || c == 0x1E2 || c == 0x1E3 || c == 0x1FC || c == 0x1FD) {
		return _('e'); // "ae" or "oe"
	} else if (c == 0x132 || c == 0x133 || (c >= 0x1C7 && c <= 0x1CC)) {
		return _('j'); // "ij", "lj", "nj"
	} else if ((c >= 0x1C4 && c <= 0x1C6) || (c >= 0x1F1 && c <= 0x1F3)) {
		return _('z'); // "dz"
	} else {
		return 0;
	}
}

int smart_compare(const String& sa, const String& sb) {
	bool in_num = false; // are we inside a number?
	bool lt = false;     // is sa less than sb?
	bool eq = true;      // so far is everything equal?
	size_t na = sa.size(), nb = sb.size();
	size_t pa = 0, pb = 0;
	for (; pa < na && pb < nb ; ++pa, ++pb) {
		Char a = sa.GetChar(pa), b = sb.GetChar(pb);
	next:
		bool da = isDigit(a), db = isDigit(b);
		if (da && db) {
			// compare numbers
			in_num = true;
			if (eq && a != b) {
				eq = false;
				lt = a < b;
			}
		} else if (in_num && da) {
			// comparing numbers, one is longer, therefore it is greater
			return 1;
		} else if (in_num && db) {
			return -1;
		} else if (in_num && !eq) {
			// two numbers of the same length, but not equal
			return lt ? -1 : 1;
		} else if (a != b) {
			// not a number
			eq = true; lt = false;
			if (a >= 0x20 && b >= 0x20) {
				// compare characters
				Char la = remove_accents(a), lb = remove_accents(b);
				// Decompose characters
				Char la2 = decompose_char2(a), lb2 = decompose_char2(b);
				// Compare
				if (la < lb) return -1;
				if (la > lb) return 1;
				// Remaining from decomposition
				if (la2 || lb2) {
					if (la2) a = la2;
					else {
						if (++pa >= na) return 1;
						a = sa.GetChar(pa);
					}
					if (lb2) b = lb2;
					else {
						if (++pb >= nb) return -1;
						b = sb.GetChar(pb);
					}
					goto next; // don't move to the next character in both strings
				}
			} else {
				// control characters
				if (a < b) return -1;
				else       return 1;
			}
		}
		in_num = da && db;
	}
	// When we are at the end; shorter strings come first
	// This is true for normal string collation
	// and also when both end in a number and another digit follows
	if (in_num) {
		if (na - pa < nb - pb) {
			// number b continues?
			Char b = sb.GetChar(pb);
			if (isDigit(b) || eq) return -1; // b is longer
		} else if (na - pa > nb - pb) {
			Char a = sa.GetChar(pa);
			if (isDigit(a) || eq) return 1;  // a is longer
		}
		return eq ? 0 : lt ? -1 : 1; // compare numbers
	} else {
		return na - pa == nb - pb ? 0
		     : na - pa <  nb - pb ? -1 : 1; // outside number, shorter string comes first
	}
}
bool smart_less(const String& sa, const String& sb) {
	return smart_compare(sa, sb) == -1;
}
bool smart_equal(const String& sa, const String& sb) {
	return smart_compare(sa, sb) == 0;
}

bool starts_with(const String& str, const String& start) {
	if (str.size() < start.size()) return false;
	FOR_EACH_2_CONST(a, str, b, start) {
		if (a != b) return false;
	}
	return true;
}

bool is_substr(const String& str, size_t pos, const Char* cmp) {
	for (String::const_iterator it = str.begin() + pos ; *cmp && it < str.end() ; ++cmp, ++it) {
		if (*cmp != *it) return false;
	}
	return *cmp == _('\0');
}
bool is_substr(const String& str, size_t pos, const String& cmp) {
	return is_substr(str, pos, cmp.c_str());
}


bool is_substr_i(const String& str, size_t pos, const Char* cmp) {
	for (String::const_iterator it = str.begin() + pos ; *cmp && it < str.end() ; ++cmp, ++it) {
		if (toLower(*cmp) != toLower(*it)) return false;
	}
	return *cmp == _('\0');
}
bool is_substr_i(const String& str, size_t pos, const String& cmp) {
	return is_substr_i(str, pos, cmp.c_str());
}

bool cannocial_name_compare(const String& as, const Char* b) {
	const Char* a = as.c_str();
	while (true) {
		if (*a != *b && !(*a == _(' ') && *b == _('_'))) return false;
		if (*a == _('\0')) return true;
		a++; b++;
	}
}

// ----------------------------------------------------------------------------- : Regular expressions

/// Escape a single character for use in regular expressions
String regex_escape(Char c) {
	if (c == _('(') || c == _(')') || c == _('[') || c == _(']') || c == _('{') ||
	    c == _('.') || c == _('^') || c == _('$') || c == _('#') || c == _('\\') ||
	    c == _('|') || c == _('+') || c == _('*') || c == _('?')) {
		// c needs to be escaped
		return _("\\") + String(1,c);
	} else {
		return String(1,c);
	}
}
/// Escape a string for use in regular expressions
String regex_escape(const String& s) {
	String ret;
	FOR_EACH_CONST(c,s) ret += regex_escape(c);
	return ret;
}

String make_non_capturing(const String& re) {
	String ret;
	bool escape = false, bracket = false, capture = false;
	FOR_EACH_CONST(c, re) {
		if (capture) {
			if (c != _('?')) {
				// change this capture into a non-capturing "(" by appending "?:"
				ret += _("?:");
			}
			capture = false;
		}
		if (escape) { // second char of escape sequence
			escape = false;
		} else if (c == _('\\')) { // start of escape sequence
			escape = true;
		} else if (c == _('[')) { // start of [...]
			bracket = true;
		} else if (c == _(']')) { // end of [...]
			bracket = false;
		} else if (bracket && c == _('(')) {
			// wx has a bug, it counts the '(' in "[(]" as a matching group
			// escape it so wx doesn't see it
			ret += _('\\');
		} else if (c == _('(')) { // start of capture?
			capture = true;
		}
		ret += c;
	}
	return ret;
}
