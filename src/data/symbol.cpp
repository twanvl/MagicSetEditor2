//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
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

// ----------------------------------------------------------------------------- : Bounds

void Bounds::update(const Vector2D& p) {
	min = piecewise_min(min, p);
	max = piecewise_max(max, p);
}
void Bounds::update(const Bounds& b) {
	min = piecewise_min(min, b.min);
	max = piecewise_max(max, b.max);
}

bool Bounds::contains(const Vector2D& p) const {
	return p.x >= min.x && p.y >= min.y &&
	       p.x <= max.x && p.y <= max.y;
}
bool Bounds::contains(const Bounds& b) const {
	return b.min.x >= min.x && b.min.y >= min.y &&
	       b.max.x <= max.x && b.max.y <= max.y;
}

Vector2D Bounds::corner(int dx, int dy) const {
	return Vector2D(
		0.5 * (min.x + max.x + dx * (max.x - min.x)),
		0.5 * (min.y + max.y + dy * (max.y - min.y)));
}

// ----------------------------------------------------------------------------- : SymbolPart

void SymbolPart::updateBounds() {
	calculateBounds(Vector2D(), Matrix2D(), true);
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


template<typename T> void fix(const T&,SymbolShape&) {}
void fix(const Reader& reader, SymbolShape& shape) {
	if (reader.file_app_version != Version()) return;
	shape.updateBounds();
	if (shape.bounds.max.x < 100 || shape.bounds.max.y < 100) return;
	// this is a <= 0.1.2 symbol, points range [0...500] instead of [0...1]
	// adjust it
	FOR_EACH(p, shape.points) {
		p->pos          /= 500.0;
		p->delta_before /= 500.0;
		p->delta_after  /= 500.0;
	}
	if (shape.name.empty()) shape.name = _("Shape");
	shape.updateBounds();
}

IMPLEMENT_REFLECTION(SymbolShape) {
	REFLECT_BASE(SymbolPart);
	REFLECT(combine);
	REFLECT(points);
	// Fixes after reading
	REFLECT_IF_READING {
		// enforce constraints
		enforceConstraints();
		fix(tag,*this);
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

Bounds SymbolShape::calculateBounds(const Vector2D& origin, const Matrix2D& m, bool is_identity) {
	Bounds bounds;
	for (int i = 0 ; i < (int)points.size() ; ++i) {
		bounds.update(segment_bounds(origin, m, *getPoint(i), *getPoint(i + 1)));
	}
	if (is_identity) this->bounds = bounds;
	return bounds;
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

Bounds SymbolSymmetry::calculateBounds(const Vector2D& origin, const Matrix2D& m, bool is_identity) {
	Bounds bounds;
	// See SymbolViewer::draw
	double b = 2 * handle.angle();
	int copies = kind == SYMMETRY_REFLECTION ? this->copies & ~1 : this->copies;
	FOR_EACH_CONST(p, parts) {
		for (int i = 0 ; i < copies ; ++i) {
			double a = i * 2 * M_PI / copies;
			if (kind == SYMMETRY_ROTATION || i % 2 == 0) {
				Matrix2D rot(cos(a),-sin(a), sin(a),cos(a));
				bounds.update(
					p->calculateBounds(origin + (center - center*rot) * m, rot * m, is_identity && i == 0)
				);
			} else {
				Matrix2D rot(cos(a+b),sin(a+b), sin(a+b),-cos(a+b));
				bounds.update(
					p->calculateBounds(origin + (center - center*rot) * m, rot * m, is_identity && i == 0)
				);
			}
		}
	}
	// done
	if (is_identity) this->bounds = bounds;
	return bounds;
}

IMPLEMENT_REFLECTION(SymbolSymmetry) {
	REFLECT_BASE(SymbolPart);
	REFLECT(kind);
	REFLECT(copies);
	REFLECT(center);
	REFLECT(handle);
	REFLECT(parts);
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

Bounds SymbolGroup::calculateBounds(const Vector2D& origin, const Matrix2D& m, bool is_identity) {
	Bounds bounds;
	FOR_EACH(p, parts) {
		bounds.update(p->calculateBounds(origin, m, is_identity));
	}
	if (is_identity) this->bounds = bounds;
	return bounds;
}

IMPLEMENT_REFLECTION(SymbolGroup) {
	REFLECT_BASE(SymbolPart);
	REFLECT(parts);
}

// ----------------------------------------------------------------------------- : Symbol

IMPLEMENT_REFLECTION(Symbol) {
	REFLECT(parts);
	REFLECT_IF_READING updateBounds();
}

double Symbol::aspectRatio() const {
	// Margin between the edges and the symbol.
	// In each direction take the lowest one
	// This is at most 0.5 (if the symbol is just a line in the middle)
	// Multiply by 2 (below) to give something in the range [0...1] i.e. [touches the edge...only in the middle]
	double margin_x = min(0.4999, max(0., min(bounds.min.x, 1-bounds.max.x)));
	double margin_y = min(0.4999, max(0., min(bounds.min.y, 1-bounds.max.y)));
	// The difference between these two,
	// e.g. if the vertical margin is more then the horizontal one, the symbol is 'flat'
	double delta = 2 * (margin_y - margin_x);
	// The aspect ratio, i.e. width/height
	if (delta > 0) {
		return 1 / (1 - delta);
	} else {
		return 1 + delta;
	}
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
