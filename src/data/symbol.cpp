//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/symbol.hpp>
#include <gfx/bezier.hpp>

// ----------------------------------------------------------------------------- : ControlPoint

IMPLEMENT_REFLECTION_ENUM(LockMode) {
	VALUE_N("free",      LOCK_FREE);
	VALUE_N("direction", LOCK_DIR);
	VALUE_N("size",      LOCK_SIZE);
}
IMPLEMENT_REFLECTION_ENUM(SegmentMode) {
	VALUE_N("line",  SEGMENT_LINE);
	VALUE_N("curve", SEGMENT_CURVE);
}
IMPLEMENT_REFLECTION(ControlPoint) {
	REFLECT_N("position",      pos);
	REFLECT_N("lock",          lock);
	REFLECT_N("line after",    segmentAfter);
	if (tag.reading() || segmentBefore == SEGMENT_CURVE) {
		REFLECT_N("handle before", deltaBefore);
	}
	if (tag.reading() || segmentAfter  == SEGMENT_CURVE) {
		REFLECT_N("handle after",  deltaAfter);
	}
}

ControlPoint::ControlPoint()
	: segmentBefore(SEGMENT_LINE), segmentAfter(SEGMENT_LINE)
	, lock(LOCK_FREE)
{}
ControlPoint::ControlPoint(double x, double y)
	: segmentBefore(SEGMENT_LINE), segmentAfter(SEGMENT_LINE)
	, lock(LOCK_FREE)
	, pos(x,y)
{}
ControlPoint::ControlPoint(double x, double y, double xb, double yb, double xa, double ya, LockMode lock)
	: segmentBefore(SEGMENT_CURVE), segmentAfter(SEGMENT_CURVE)
	, lock(lock)
	, pos(x,y)
	, deltaBefore(xb,yb)
	, deltaAfter(xa,ya)
{}

void ControlPoint::onUpdateHandle(WhichHandle wh) {
	// One handle has changed, update only the other one
	if (lock == LOCK_DIR) {
		getOther(wh) = -getHandle(wh) * getOther(wh).length() / getHandle(wh).length();
	} else if (lock == LOCK_SIZE) {
		getOther(wh) = -getHandle(wh);
	}
}
void ControlPoint::onUpdateLock() {
	// The lock has changed, avarage the handle values
	if (lock == LOCK_DIR) {
		// deltaBefore = x * deltaAfter
		Vector2D dir = (deltaBefore - deltaAfter).normalized();
		deltaBefore = dir * deltaBefore.length();
		deltaAfter  = dir * -deltaAfter.length();
	} else if (lock == LOCK_SIZE) {
		// deltaBefore = -deltaAfter
		deltaBefore = (deltaBefore - deltaAfter) * 0.5;
		deltaAfter  = -deltaBefore;
	}
}

Vector2D& ControlPoint::getHandle(WhichHandle wh) {
	if (wh == HANDLE_BEFORE) {
		return deltaBefore;
	} else {
		assert(wh == HANDLE_AFTER);
		return deltaAfter;
	}
}
Vector2D& ControlPoint::getOther(WhichHandle wh) {
	if (wh == HANDLE_BEFORE) {
		return deltaAfter;
	} else {
		assert(wh == HANDLE_AFTER);
		return deltaBefore;
	}
}

// ----------------------------------------------------------------------------- : SymbolPart

IMPLEMENT_REFLECTION_ENUM(SymbolPartCombine) {
	VALUE_N("overlap",		PART_OVERLAP);
	VALUE_N("merge",		PART_MERGE);
	VALUE_N("subtract",		PART_SUBTRACT);
	VALUE_N("intersection",	PART_INTERSECTION);
	VALUE_N("difference",	PART_DIFFERENCE);
	VALUE_N("border",		PART_BORDER);
}

IMPLEMENT_REFLECTION(SymbolPart) {
	REFLECT(name);
	REFLECT(combine);
	REFLECT_N("point", points);
	// Fixes after reading
	if (tag.reading()) {
		// enforce constraints
		enforceConstraints();
		calculateBounds();
		if (maxPos.x > 100 && maxPos.y > 100) {
			// this is a <= 0.1.2 symbol, points range [0...500] instead of [0...1]
			// adjust it
			FOR_EACH(p, points) {
				p->pos         /= 500.0;
				p->deltaBefore /= 500.0;
				p->deltaAfter  /= 500.0;
			}
			if (name.empty()) name = _("Shape");
			calculateBounds();
		}
	}
}

SymbolPart::SymbolPart()
	: combine(PART_OVERLAP), rotationCenter(.5, .5)
{}

SymbolPartP SymbolPart::clone() const {
	SymbolPartP part = new_shared1<SymbolPart>(*this);
	// also clone the control points
	FOR_EACH(p, part->points) {
		p = new_shared1<ControlPoint>(*p);
	}
	return part;
}

void SymbolPart::enforceConstraints() {
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		ControlPointP p1 = getPoint(i);
		ControlPointP p2 = getPoint(i + 1);
		p2->segmentBefore = p1->segmentAfter;
		p1->onUpdateLock();
	}
}

void SymbolPart::calculateBounds() {
	minPos =  Vector2D::infinity();
	maxPos = -Vector2D::infinity();
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		segmentBounds(*getPoint(i), *getPoint(i + 1), minPos, maxPos);
	}
}

// ----------------------------------------------------------------------------- : Symbol

IMPLEMENT_REFLECTION(Symbol) {
//%% version?
	REFLECT_N("part", parts);
}

// ----------------------------------------------------------------------------- : SymbolView

SymbolView::SymbolView() {}

SymbolView::~SymbolView() {
	if (symbol) symbol->actions.removeListener(this);
}

void SymbolView::setSymbol(const SymbolP& newSymbol) {
	// no longer listening to old symbol
	if (symbol) symbol->actions.removeListener(this);
	symbol = newSymbol;
	// start listening to new symbol
	if (symbol) symbol->actions.addListener(this);
	onChangeSymbol();
}
