//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
		outer.x + align_delta_x(align, outer.width,  to_align.width),
		outer.y + align_delta_y(align, outer.height, to_align.height)
	);
}

// ----------------------------------------------------------------------------- : Reflection stuff

/// Convert a String to an Alignment
Alignment from_string(const String& s) {
	int al = ALIGN_TOP_LEFT;
	if (s.find(_("left"))             !=String::npos) al = ALIGN_LEFT             | (al & ~ALIGN_HORIZONTAL);
	if (s.find(_("center"))           !=String::npos) al = ALIGN_CENTER           | (al & ~ALIGN_HORIZONTAL);
	if (s.find(_("right"))            !=String::npos) al = ALIGN_RIGHT            | (al & ~ALIGN_HORIZONTAL);
	if (s.find(_("justify"))          !=String::npos) al = ALIGN_JUSTIFY          | (al & ~ALIGN_HORIZONTAL);
	if (s.find(_("justify-words"))    !=String::npos) al = ALIGN_JUSTIFY_WORDS    | (al & ~ALIGN_HORIZONTAL);
	if (s.find(_("justify-overflow")) !=String::npos) al = ALIGN_JUSTIFY_OVERFLOW | (al & ~ALIGN_JUSTIFY_OVERFLOW);
	if (s.find(_("shrink-overflow"))  !=String::npos) al = ALIGN_STRETCH_OVERFLOW | (al & ~ALIGN_STRETCH_OVERFLOW); // compatability
	if (s.find(_("stretch-overflow")) !=String::npos) al = ALIGN_STRETCH_OVERFLOW | (al & ~ALIGN_STRETCH_OVERFLOW);
	else if (s.find(_("stretch"))     !=String::npos) al = ALIGN_STRETCH;
	if (s.find(_("top"))              !=String::npos) al = ALIGN_TOP              | (al & ~ALIGN_VERTICAL);
	if (s.find(_("middle"))           !=String::npos) al = ALIGN_MIDDLE           | (al & ~ALIGN_VERTICAL);
	if (s.find(_("bottom"))           !=String::npos) al = ALIGN_BOTTOM           | (al & ~ALIGN_VERTICAL);
	return static_cast<Alignment>(al);
}

/// Convert an Alignment to a String
String to_string(Alignment align) {
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
	if (align & ALIGN_JUSTIFY_OVERFLOW) ret += _(" justify-overflow");
	// modifier
	if (align & ALIGN_STRETCH_OVERFLOW) ret += _(" stretch-overflow");
	if (align & ALIGN_STRETCH)			ret += _(" stretch");
	return ret.substr(1);
}

// we need custom io, because there can be both a horizontal and a vertical component

template <> void Reader::handle(Alignment& align) {
	align = from_string(getValue());
}
template <> void Writer::handle(const Alignment& align) {
	handle(to_string(align));
}
template <> void GetDefaultMember::handle(const Alignment& align) {
	handle(to_string(align));
}

// ----------------------------------------------------------------------------- : Direction

IMPLEMENT_REFLECTION_ENUM(Direction) {
	VALUE_N("left to right", LEFT_TO_RIGHT);
	VALUE_N("right to left", RIGHT_TO_LEFT);
	VALUE_N("top to bottom", TOP_TO_BOTTOM);
	VALUE_N("bottom to top", BOTTOM_TO_TOP);
	VALUE_N("horizontal",    LEFT_TO_RIGHT);
	VALUE_N("vertical",      TOP_TO_BOTTOM);
	VALUE_N("top-right to bottom-left", TOP_RIGHT_TO_BOTTOM_LEFT);
	VALUE_N("bottom-left to top-right", BOTTOM_LEFT_TO_TOP_RIGHT);
	VALUE_N("top-left to bottom-right", TOP_LEFT_TO_BOTTOM_RIGHT);
	VALUE_N("bottom-right to top-left", BOTTOM_RIGHT_TO_TOP_LEFT);
}

RealPoint move_in_direction(Direction dir, const RealPoint& point, const RealSize to_move, double spacing) {
	switch (dir) {
		case LEFT_TO_RIGHT: return RealPoint(point.x + to_move.width + spacing, point.y);
		case RIGHT_TO_LEFT: return RealPoint(point.x - to_move.width - spacing, point.y);
		case TOP_TO_BOTTOM: return RealPoint(point.x, point.y + to_move.height + spacing);
		case BOTTOM_TO_TOP: return RealPoint(point.x, point.y - to_move.height - spacing);
		case TOP_RIGHT_TO_BOTTOM_LEFT: return RealPoint(point.x - to_move.width - spacing, point.y + to_move.height + spacing);
		case BOTTOM_LEFT_TO_TOP_RIGHT: return RealPoint(point.x + to_move.width + spacing, point.y - to_move.height - spacing);
		case TOP_LEFT_TO_BOTTOM_RIGHT: return RealPoint(point.x + to_move.width + spacing, point.y + to_move.height + spacing);
		case BOTTOM_RIGHT_TO_TOP_LEFT: return RealPoint(point.x - to_move.width - spacing, point.y - to_move.height - spacing);
		default:            return point; // should not happen
	}
}
