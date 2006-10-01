//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "string.hpp"
#include "for_each.hpp"
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

String trimLeft(const String& s) {
	size_t start = s.find_first_not_of(_(' '));
	if (start == String::npos) {
		return String();
	} else {
		return s.substr(start);
	}
}

// ----------------------------------------------------------------------------- : Words

String lastWord(const String& s) {
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

String stripLastWord(const String& s) {
	size_t endLastWord   = s.find_last_not_of(_(' '));
	size_t startLastWord = s.find_last_of(_(' '), endLastWord);
	if (endLastWord == String::npos || startLastWord == String::npos) {
		return String(); // single word or empty string
	} else {
		return s.substr(0, startLastWord + 1);
	}
}

// ----------------------------------------------------------------------------- : Caseing

/// Quick check to see if the substring starting at the given iterator is equal
/// to some given string
bool is_substr(const String& s, String::iterator it, const Char* cmp) {
	while (it != s.end() && *cmp != 0) {
		if (*it++ != *cmp++) return false;
	}
	return *cmp == 0;
}
String capitalize(const String& s) {
	String result = s;
	bool afterSpace = true;
	FOR_EACH_IT(it, result) {
		if (*it == ' ') {
			afterSpace = true;
		} else if (afterSpace) {
			afterSpace = false;
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
