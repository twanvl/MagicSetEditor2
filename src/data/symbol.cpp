//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/symbol.hpp>
#include <script/to_value.hpp>
#include <gfx/bezier.hpp>

DECLARE_TYPEOF_COLLECTION(ControlPointP);
DECLARE_TYPEOF_COLLECTION(SymbolPartP);

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

void SymbolPart::calculateBounds() {
	min_pos =  Vector2D::infinity();
	max_pos = -Vector2D::infinity();
}

IMPLEMENT_REFLECTION(SymbolPart) {
	REFLECT_IF_NOT_READING {
		String type = typeName();
		REFLECT(type);
	}
	REFLECT(name);
}

template <>
SymbolPartP read_new<SymbolPart>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
	if      (type == _("shape") || type.empty())	return new_intrusive<SymbolShape>();
	else if (type == _("symmetry"))					return new_intrusive<SymbolSymmetry>();
	else if (type == _("group"))					return new_intrusive<SymbolGroup>();
	else {
		throw ParseError(_("Unsupported symbol part type: '") + type + _("'"));
	}
}

// ----------------------------------------------------------------------------- : SymbolShape

IMPLEMENT_REFLECTION_ENUM(SymbolShapeCombine) {
	VALUE_N("overlap",		SYMBOL_COMBINE_OVERLAP);
	VALUE_N("merge",		SYMBOL_COMBINE_MERGE);
	VALUE_N("subtract",		SYMBOL_COMBINE_SUBTRACT);
	VALUE_N("intersection",	SYMBOL_COMBINE_INTERSECTION);
	VALUE_N("difference",	SYMBOL_COMBINE_DIFFERENCE);
	VALUE_N("border",		SYMBOL_COMBINE_BORDER);
}

IMPLEMENT_REFLECTION(SymbolShape) {
	REFLECT_BASE(SymbolPart);
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

SymbolShape::SymbolShape()
	: combine(SYMBOL_COMBINE_OVERLAP), rotation_center(.5, .5)
{}

String SymbolShape::typeName() const {
	return _("shape");
}

SymbolPartP SymbolShape::clone() const {
	SymbolShapeP part(new SymbolShape(*this));
	// also clone the control points
	FOR_EACH(p, part->points) {
		p = new_intrusive1<ControlPoint>(*p);
	}
	return part;
}

void SymbolShape::enforceConstraints() {
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		ControlPointP p1 = getPoint(i);
		ControlPointP p2 = getPoint(i + 1);
		p2->segment_before = p1->segment_after;
		p1->onUpdateLock();
	}
}

void SymbolShape::calculateBounds() {
	min_pos =  Vector2D::infinity();
	max_pos = -Vector2D::infinity();
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		segment_bounds(*getPoint(i), *getPoint(i + 1), min_pos, max_pos);
	}
}

// ----------------------------------------------------------------------------- : SymbolSymmetry

IMPLEMENT_REFLECTION_ENUM(SymbolSymmetryType) {
	VALUE_N("rotation",   SYMMETRY_ROTATION);
	VALUE_N("reflection", SYMMETRY_REFLECTION);
}

SymbolSymmetry::SymbolSymmetry()
	: kind(SYMMETRY_ROTATION), copies(2)
{}

String SymbolSymmetry::typeName() const {
	return _("symmetry");
}

SymbolPartP SymbolSymmetry::clone() const {
	SymbolSymmetryP part(new SymbolSymmetry(*this));
	// also clone the parts inside
	FOR_EACH(p, part->parts) {
		p = p->clone();
	}
	return part;
}

String SymbolSymmetry::expectedName() const {
	return capitalize(kind == SYMMETRY_ROTATION ? _TYPE_("rotation") : _TYPE_("reflection"))
	     + String::Format(_(" (%d)"), copies);
}

IMPLEMENT_REFLECTION(SymbolSymmetry) {
	REFLECT_BASE(SymbolPart);
	REFLECT(kind);
	REFLECT(copies);
	REFLECT(center);
	REFLECT(handle);
	REFLECT(parts);
	REFLECT_IF_READING calculateBoundsNonRec();
}

// ----------------------------------------------------------------------------- : SymbolGroup

SymbolGroup::SymbolGroup() {
	name = capitalize(_TYPE_("group"));
}

String SymbolGroup::typeName() const {
	return _("group");
}

SymbolPartP SymbolGroup::clone() const {
	SymbolGroupP part(new SymbolGroup(*this));
	// also clone the parts inside
	FOR_EACH(p, part->parts) {
		p = p->clone();
	}
	return part;
}

bool SymbolGroup::isAncestor(const SymbolPart& that) const {
	if (this == &that) return true;
	FOR_EACH_CONST(p, parts) {
		if (p->isAncestor(that)) return true;
	}
	return false;
}

void SymbolGroup::calculateBounds() {
	FOR_EACH(p, parts) p->calculateBounds();
	calculateBoundsNonRec();
}
void SymbolGroup::calculateBoundsNonRec() {
	min_pos =  Vector2D::infinity();
	max_pos = -Vector2D::infinity();
	FOR_EACH(p, parts) {
		min_pos = piecewise_min(min_pos, p->min_pos);
		max_pos = piecewise_max(max_pos, p->max_pos);
	}
}

IMPLEMENT_REFLECTION(SymbolGroup) {
	REFLECT_BASE(SymbolPart);
	REFLECT(parts);
	REFLECT_IF_READING calculateBoundsNonRec();
}

// ----------------------------------------------------------------------------- : Symbol

IMPLEMENT_REFLECTION(Symbol) {
	REFLECT(parts);
	REFLECT_IF_READING calculateBoundsNonRec();
}

// ----------------------------------------------------------------------------- : Default symbol

// A default symbol part, a square, moved by d
SymbolShapeP default_symbol_part(double d) {
	SymbolShapeP part = new_intrusive<SymbolShape>();
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
