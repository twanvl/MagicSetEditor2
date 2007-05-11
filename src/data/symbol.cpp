//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/symbol.hpp>
#include <script/to_value.hpp>
#include <gfx/bezier.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolPartP);
DECLARE_TYPEOF_COLLECTION(ControlPointP);

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
	REFLECT_N("line_after",    segment_after);
	if (tag.reading() || segment_before == SEGMENT_CURVE) {
		REFLECT_N("handle_before", delta_before);
	}
	if (tag.reading() || segment_after  == SEGMENT_CURVE) {
		REFLECT_N("handle_after",  delta_after);
	}
}

ControlPoint::ControlPoint()
	: segment_before(SEGMENT_LINE), segment_after(SEGMENT_LINE)
	, lock(LOCK_FREE)
{}
ControlPoint::ControlPoint(double x, double y)
	: pos(x,y)
	, segment_before(SEGMENT_LINE), segment_after(SEGMENT_LINE)
	, lock(LOCK_FREE)
{}
ControlPoint::ControlPoint(double x, double y, double xb, double yb, double xa, double ya, LockMode lock)
	: pos(x,y)
	, delta_before(xb,yb)
	, delta_after(xa,ya)
	, segment_before(SEGMENT_CURVE), segment_after(SEGMENT_CURVE)
	, lock(lock)
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
		// delta_before = x * delta_after
		Vector2D dir = (delta_before - delta_after).normalized();
		delta_before = dir * delta_before.length();
		delta_after  = dir * -delta_after.length();
	} else if (lock == LOCK_SIZE) {
		// delta_before = -delta_after
		delta_before = (delta_before - delta_after) * 0.5;
		delta_after  = -delta_before;
	}
}

Vector2D& ControlPoint::getHandle(WhichHandle wh) {
	if (wh == HANDLE_BEFORE) {
		return delta_before;
	} else {
		assert(wh == HANDLE_AFTER);
		return delta_after;
	}
}
Vector2D& ControlPoint::getOther(WhichHandle wh) {
	if (wh == HANDLE_BEFORE) {
		return delta_after;
	} else {
		assert(wh == HANDLE_AFTER);
		return delta_before;
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
	REFLECT(points);
	// Fixes after reading
	REFLECT_IF_READING {
		// enforce constraints
		enforceConstraints();
		calculateBounds();
		if (max_pos.x > 100 && max_pos.y > 100) {
			// this is a <= 0.1.2 symbol, points range [0...500] instead of [0...1]
			// adjust it
			FOR_EACH(p, points) {
				p->pos          /= 500.0;
				p->delta_before /= 500.0;
				p->delta_after  /= 500.0;
			}
			if (name.empty()) name = _("Shape");
			calculateBounds();
		}
	}
}

SymbolPart::SymbolPart()
	: combine(PART_OVERLAP), rotation_center(.5, .5)
{}

SymbolPartP SymbolPart::clone() const {
	SymbolPartP part = new_intrusive1<SymbolPart>(*this);
	// also clone the control points
	FOR_EACH(p, part->points) {
		p = new_intrusive1<ControlPoint>(*p);
	}
	return part;
}

void SymbolPart::enforceConstraints() {
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		ControlPointP p1 = getPoint(i);
		ControlPointP p2 = getPoint(i + 1);
		p2->segment_before = p1->segment_after;
		p1->onUpdateLock();
	}
}

void SymbolPart::calculateBounds() {
	min_pos =  Vector2D::infinity();
	max_pos = -Vector2D::infinity();
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		segment_bounds(*getPoint(i), *getPoint(i + 1), min_pos, max_pos);
	}
}

// ----------------------------------------------------------------------------- : Symbol

IMPLEMENT_REFLECTION(Symbol) {
	REFLECT(parts);
}

// ----------------------------------------------------------------------------- : Default symbol

// A default symbol part, a square, moved by d
SymbolPartP default_symbol_part(double d) {
	SymbolPartP part = new_intrusive<SymbolPart>();
	part->points.push_back(new_intrusive2<ControlPoint>(d + .2, d + .2));
	part->points.push_back(new_intrusive2<ControlPoint>(d + .2, d + .8));
	part->points.push_back(new_intrusive2<ControlPoint>(d + .8, d + .8));
	part->points.push_back(new_intrusive2<ControlPoint>(d + .8, d + .2));
	part->name = _("Square");
	return part;
}

// A default symbol, a square
SymbolP default_symbol() {
	SymbolP symbol = new_intrusive<Symbol>();
	symbol->parts.push_back(default_symbol_part(0));
	return symbol;
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
