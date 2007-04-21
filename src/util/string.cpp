//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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

// ----------------------------------------------------------------------------- : String utilities

String trim(const String& s){
	size_t start = s.find_first_not_of(_(" \t"));
	size_t end   = s.find_last_not_of( _(" \t"));
	if (start == String::npos) {
		return String();
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
	String ret = s.Lower();
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
			if (!leading) ret += _(' ');
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

bool smart_less(const String& as, const String& bs) {
	bool in_num = false; // are we inside a number?
	bool lt = false;     // is as less than bs?
	bool eq = true;      // so far is everything equal?
	FOR_EACH_2_CONST(a, as, b, bs) {
		bool na = isDigit(a), nb = isDigit(b);
		Char la = toLower(a), lb = toLower(b);
		if (na && nb) {
			// compare numbers
			in_num = true;
			if (eq && a != b) {
				eq = false;
				lt = a < b;
			}
		} else if (in_num && na) {
			// comparing numbers, one is longer, therefore it is greater
			return false;
		} else if (in_num && nb) {
			return true;
		} else if (in_num && !eq) {
			// two numbers of the same length, but not equal
			return lt;
		} else {
			// compare characters
			if (la < lb)  return true;
			if (la > lb)  return false;
		}
		in_num = na && nb;
	}
	// When we are at the end; shorter strings come first
	// This is true for normal string collation
	// and also when both end in a number and another digit follows
	if (as.size() != bs.size()) {
		return as.size() < bs.size();
	} else {
		return lt;
	}
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

bool cannocial_name_compare(const String& as, const Char* b) {
	const Char* a = as.c_str();
	while (true) {
		if (*a != *b && !(*a == _(' ') && *b == _('_'))) return false;
		if (*a == _('\0')) return true;
		a++; b++;
	}
}
