//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/tagged_string.hpp>

// ----------------------------------------------------------------------------- : Conversion to/from normal string

String untag(const String& str) {
	bool intag = false;
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (c==_('<')) intag = true;
		if (!intag) ret += c==_('\1') ? _('<') : c;
		if (c==_('>')) intag = false;
	}
	return ret;
}

String untag_no_escape(const String& str) {
	bool intag = false;
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (c==_('<')) intag = true;
		if (!intag) ret += c;
		if (c==_('>')) intag = false;
	}
	return ret;
}

String escape(const String& str) {
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (c==_('<')) ret += _('\1');
		else           ret += c;
	}
	return ret;
}

// ----------------------------------------------------------------------------- : Finding tags

size_t skip_tag(const String& str, size_t start) {
	if (start >= str.size()) return String::npos;
	size_t end = str.find_first_of(_('>'), start);
	return end == String::npos ? String::npos : end + 1;
}

// ----------------------------------------------------------------------------- : Global operations
// ----------------------------------------------------------------------------- : Simplification
