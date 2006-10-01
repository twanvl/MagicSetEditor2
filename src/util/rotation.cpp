//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/rotation.hpp>

// ----------------------------------------------------------------------------- : Rotation

Rotation::Rotation(int angle, const RealRect& rect, double zoom)
	: angle(angle)
	, size(rect.size)
	, origin(rect.position)
	, zoom(zoom)
{
	// set origin
	if (revX()) origin.x += size.width;
	if (revY()) origin.x += size.height;
}

RealPoint Rotation::tr(const RealPoint& p) const {
	return tr(RealSize(p.x, p.y)) + origin; // TODO : optimize?
}
RealSize Rotation::tr(const RealSize& s) const {
	if (sideways()) {
		return RealSize(negX(s.height), negY(s.width)) * zoom;
	} else {
		return RealSize(negX(s.width), negY(s.height)) * zoom;
	}
}
RealRect Rotation::tr(const RealRect& r) const {
	return RealRect(tr(r.position), tr(r.size));
}

RealSize Rotation::trNoNeg(const RealSize& s) const {
	if (sideways()) {
		return RealSize(s.height, s.width) * zoom;
	} else {
		return RealSize(s.width, s.height) * zoom;
	}
}
RealRect Rotation::trNoNeg(const RealRect& r) const {
	throw "TODO";
}

RealPoint Rotation::trInv(const RealPoint& p) const {
	RealPoint p2 = (p - origin) / zoom;
	if (sideways()) {
		return RealPoint(negY(p2.y), negX(p2.x));
	} else {
		return RealPoint(negX(p2.x), negY(p2.y));
	}
}