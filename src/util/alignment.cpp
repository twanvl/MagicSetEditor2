//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/alignment.hpp>
#include <util/reflect.hpp>

// ----------------------------------------------------------------------------- : Alignment

double align_delta_x(Alignment align, double box_width, double obj_width) {
	if      (align & ALIGN_CENTER) return (box_width - obj_width) / 2;
	else if (align & ALIGN_RIGHT)  return  box_width - obj_width;
	else                           return 0;
}


double align_delta_y(Alignment align, double box_height, double obj_height) {
	if      (align & ALIGN_MIDDLE) return (box_height - obj_height) / 2;
	else if (align & ALIGN_BOTTOM) return  box_height - obj_height;
	else                           return 0;
}

RealPoint align_in_rect(Alignment align, const RealSize& to_align, const RealRect& outer) {
	return RealPoint(
		outer.position.x + align_delta_x(align, outer.size.width,  to_align.width),
		outer.position.y + align_delta_y(align, outer.size.height, to_align.height)
	);
}

// ----------------------------------------------------------------------------- : Reflection stuff

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
template <> void GetDefaultMember::handle(const Alignment& align) {
	handle(toString(align));
}
