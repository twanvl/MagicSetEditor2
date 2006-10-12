//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/alignment.hpp>
#include <util/reflect.hpp>

// ----------------------------------------------------------------------------- : Alignment

/// Convert a String to an Alignment
Alignment fromString(const String& str) {
	int al = 0;
	return static_cast<Alignment>(al);
}

/// Convert an Alignment to a String
String toString(Alignment align) {
	String ret;
	// vertical
	if (align & ALIGN_TOP)				ret += _(" top");
	if (align & ALIGN_MIDDLE)			ret += _(" middle");
	if (align & ALIGN_BOTTOM)			ret += _(" bottom");
	// horizontal
	if (align & ALIGN_LEFT)				ret += _(" left");
	if (align & ALIGN_LEFT)				ret += _(" center");
	if (align & ALIGN_LEFT)				ret += _(" right");
	if (align & ALIGN_LEFT)				ret += _(" justify");
	if (align & ALIGN_LEFT)				ret += _(" justify-words");
	// modifier
	if (align & ALIGN_JUSTIFY_OVERFLOW) ret += _(" shrink-overflow");
	if (align & ALIGN_STRETCH)			ret += _(" stretch");
	return ret.substr(1);
}

// we need custom io, because there can be both a horizontal and a vertical component

template <> void Reader::handle(Alignment& align) {
	align = fromString(value);
}
template <> void Writer::handle(const Alignment& align) {
	handle(toString(align));
}
template <> void GetMember::store(const Alignment& align) {
	store(toString(align));
}
