//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/action/symbol_part.hpp>
#include <gfx/bezier.hpp>

DECLARE_TYPEOF_COLLECTION(Vector2D);
DECLARE_TYPEOF_COLLECTION(ControlPointP);

// ----------------------------------------------------------------------------- : Utility

inline double sgn(double v) { return  v > 0 ? 1 : -1; }

Vector2D constrain_vector(const Vector2D& v, bool constrain, bool only_diagonal) {
	if (!constrain) return v;
	double ax = fabs(v.x), ay = fabs(v.y);
	if (ax * 2 < ay && !only_diagonal) {
		return Vector2D(0, v.y); // vertical
	} else if(ay * 2 < ax && !only_diagonal) {
		return Vector2D(v.x, 0); // horizontal
	} else {
		return Vector2D(         // diagonal
			sgn(v.x) * (ax + ay) / 2,
			sgn(v.y) * (ax + ay) / 2
		);
	}
}

inline double snap(double x, int steps) {
	return steps <= 0 ? x : floor(x * steps + 0.5) / steps;
}

Vector2D snap_vector(const Vector2D& v, int steps) {
	return Vector2D(snap(v.x, steps), snap(v.y, steps));
}

Vector2D constrain_snap_vector(const Vector2D& v, const Vector2D& d, bool constrain, int steps) {
	if (!constrain) return snap_vector(v+d, steps);
	double ax = fabs(d.x), ay = fabs(d.y);
	if (ax * 2 < ay) {
		return Vector2D(v.x, snap(d.y + v.y, steps)); // vertical
	} else if(ay * 2 < ax) {
		return Vector2D(snap(d.x + v.x, steps), v.y); // horizontal
	} else {	
		double dc = (ax + ay) / 2; // delta in both directions
		double dxs = snap(v.x + dc, steps) - v.x; // snapped to x
		double dys = snap(v.y + dc, steps) - v.y; // snapped to y
		if (fabs(dxs-dc) < fabs(dys-dc)) {
			// take the one that is closest to the unsnaped delta
			return Vector2D(v.x + sgn(d.x) * dxs, v.y + sgn(d.y) * dxs);
		} else {
			return Vector2D(v.x + sgn(d.x) * dys, v.y + sgn(d.y) * dys);
		}
	}
}

Vector2D constrain_snap_vector_offset(const Vector2D& off1, const Vector2D& d, bool constrain, int steps) {
	return constrain_snap_vector(off1, d, constrain, steps) - off1;
}
// calculate constrained delta for the given offset, store in output if it is better
void constrain_snap_vector_offset_(const Vector2D& off, const Vector2D& d, bool constrain, int steps, Vector2D& best, double& best_length) {
	Vector2D d2 = constrain_snap_vector_offset(off, d, constrain, steps);
	double l2 = d2.lengthSqr();
	if (l2 < best_length) {
		best_length = l2;
		best        = d2;
	}
}
Vector2D constrain_snap_vector_offset(const Vector2D& off1, const Vector2D& off2, const Vector2D& d, bool constrain, int steps) {
	Vector2D dd; double l = numeric_limits<double>::infinity();
	constrain_snap_vector_offset_(off1,                    d, constrain, steps, dd, l);
	constrain_snap_vector_offset_(off2,                    d, constrain, steps, dd, l);
	constrain_snap_vector_offset_(Vector2D(off1.x,off2.y), d, constrain, steps, dd, l);
	constrain_snap_vector_offset_(Vector2D(off2.x,off1.y), d, constrain, steps, dd, l);
	return dd;
}

String action_name_for(const set<ControlPointP>& points, const String& action) {
	return format_string(action, points.size() == 1 ? _TYPE_("point") : _TYPE_("points"));
}


// ----------------------------------------------------------------------------- : Move control point

ControlPointMoveAction::ControlPointMoveAction(const set<ControlPointP>& points)
	: points(points)
	, constrain(false)
	, snap(0)
{
	oldValues.reserve(points.size());
	FOR_EACH(p, points) {
		oldValues.push_back(p->pos);
	}
}

String ControlPointMoveAction::getName(bool to_undo) const {
	return action_name_for(points, _ACTION_("move"));
}

void ControlPointMoveAction::perform(bool to_undo) {
	FOR_EACH_2(p,points,  op,oldValues) {
		swap(p->pos, op);
	}
}

void ControlPointMoveAction::move (const Vector2D& deltaDelta) {
	delta += deltaDelta;
	// Move each point by delta, possibly constrained
	set<ControlPointP>::const_iterator it  = points.begin();
	vector<Vector2D>  ::iterator       it2 = oldValues.begin();
	for( ; it != points.end() && it2 != oldValues.end() ; ++it, ++it2) {
		(*it)->pos = constrain_snap_vector(*it2, delta, constrain, snap);
	}
}


// ----------------------------------------------------------------------------- : Move handle

HandleMoveAction::HandleMoveAction(const SelectedHandle& handle)
	: handle(handle)
	, old_handle(handle.getHandle())
	, old_other (handle.getOther())
	, constrain(false)
	, snap(0)
{}

String HandleMoveAction::getName(bool to_undo) const {
	return _ACTION_("move handle");
}

void HandleMoveAction::perform(bool to_undo) {
	swap(old_handle, handle.getHandle());
	swap(old_other,  handle.getOther());
}

void HandleMoveAction::move(const Vector2D& deltaDelta) {
	delta += deltaDelta;
	handle.getHandle() = constrain_snap_vector_offset(handle.point->pos, old_handle + delta, constrain, snap);
	handle.getOther()  = old_other;
	handle.onUpdateHandle();
}


// ----------------------------------------------------------------------------- : Segment mode

ControlPointUpdate::ControlPointUpdate(const ControlPointP& pnt)
	: other(*pnt)
	, point(pnt)
{}

void ControlPointUpdate::perform() {
	swap(other, *point);
}


SegmentModeAction::SegmentModeAction(const ControlPointP& p1, const ControlPointP& p2, SegmentMode mode)
	: point1(p1), point2(p2)
{
	if (p1->segment_after == mode) return;
	point1.other.segment_after = point2.other.segment_before = mode;
	if (mode == SEGMENT_LINE) {
		point1.other.delta_after  = Vector2D(0,0);
		point2.other.delta_before = Vector2D(0,0);
		point1.other.lock = LOCK_FREE;
		point2.other.lock = LOCK_FREE;
	} else if (mode == SEGMENT_CURVE) {
		point1.other.delta_after  = (p2->pos - p1->pos) / 3.0f;
		point2.other.delta_before = (p1->pos - p2->pos) / 3.0f;
	}
}
String SegmentModeAction::getName(bool to_undo) const {
	SegmentMode mode = to_undo ? point1.point->segment_after : point1.other.segment_after;
	if (mode == SEGMENT_LINE) return _ACTION_("convert to line");
	else                      return _ACTION_("convert to curve");
}

void SegmentModeAction::perform(bool to_undo) {
	point1.perform();
	point2.perform();
}


// ----------------------------------------------------------------------------- : Locking mode

LockModeAction::LockModeAction(const ControlPointP& p, LockMode lock)
	: point(p)
{
	point.other.lock = lock;
	point.other.onUpdateLock();
}

String LockModeAction::getName(bool to_undo) const {
	return _ACTION_("lock point");
}

void LockModeAction::perform(bool to_undo) {
	point.perform();
}


// ----------------------------------------------------------------------------- : Move curve

CurveDragAction::CurveDragAction(const ControlPointP& point1, const ControlPointP& point2)
	: SegmentModeAction(point1, point2, SEGMENT_CURVE)
{}

String CurveDragAction::getName(bool to_undo) const {
	return _ACTION_("move curve");
}

void CurveDragAction::perform(bool to_undo) {
	SegmentModeAction::perform(to_undo);
}

void CurveDragAction::move(const Vector2D& delta, double t) {
	// Logic:
	//   Assuming old point is p, new point is p'
	//   Point on old bezier curve is:
	//     p  = a t^3 + 3b  (1-t) t^2 + 3c  (1-t)^2 t + d (1-t)^2
	//   Point on new bezier curve is:
	//     p_(' = a t^3 + 3b') (1-t) t^2 + 3c' (1-t)^2 t + d (1-t)^2
	//   We now want to change control points b and c, the closer we are to b (t close to 0)
	//   the more effect we have on b, so we substitute:
	//     b' = b + x t
	//     c' = c + x (1-t)
	//   Solving for x we get:
	//     x = (p'-p) / ( t (1-t) ( t^2 + (1-t)^2) ) 
	// Naming:
	//   delta    = p' - p
	//   pointDelta = x * t * (1-t)
	Vector2D pointDelta = delta / (3 * (t * t + (1-t) * (1-t)));
	point1.point->delta_after  += pointDelta / t;
	point2.point->delta_before += pointDelta / (1-t);
	point1.point->onUpdateHandle(HANDLE_AFTER);
	point2.point->onUpdateHandle(HANDLE_BEFORE);
}


// ----------------------------------------------------------------------------- : Add control point

ControlPointAddAction::ControlPointAddAction(const SymbolShapeP& shape, UInt insert_after, double t)
	: shape(shape)
	, new_point(new ControlPoint())
	, insert_after(insert_after)
	, point1(shape->getPoint(insert_after))
	, point2(shape->getPoint(insert_after + 1))
{
	// calculate new point
	if (point1.other.segment_after == SEGMENT_CURVE) {
		// calculate new handles using de Casteljau's subdivision algorithm
		deCasteljau(point1.other, point2.other, t, *new_point);
		// unlock if needed
		if (point1.other.lock == LOCK_SIZE) point1.other.lock = LOCK_DIR;
		if (point2.other.lock == LOCK_SIZE) point2.other.lock = LOCK_DIR;
		new_point->lock = LOCK_DIR;
		new_point->segment_before = SEGMENT_CURVE;
		new_point->segment_after  = SEGMENT_CURVE;
	} else {
		new_point->pos = point1.other.pos * (1 - t) + point2.other.pos * t;
		new_point->lock = LOCK_FREE;
		new_point->segment_before = SEGMENT_LINE;
		new_point->segment_after  = SEGMENT_LINE;
	}
}

String ControlPointAddAction::getName(bool to_undo) const {
	return _ACTION_("add control point");
}

void ControlPointAddAction::perform(bool to_undo) {
	if (to_undo) { // remove the point
		shape->points.erase( shape->points.begin() + insert_after + 1);
	} else {
		shape->points.insert(shape->points.begin() + insert_after + 1, new_point);
	}
	// update points before/after
	point1.perform();
	point2.perform();
}

// ----------------------------------------------------------------------------- : Remove control point

/// Sqaure root that caries the sign over the root
/// or formally: ssqrt(x) = Re<sqrt(x)> - Im<sqrt(x)> = x / sqrt(|x|)
double ssqrt(double x) {
	if (x > 0) return sqrt(x);
	else       return -sqrt(-x);
}

// Remove a single control point
class SinglePointRemoveAction : public Action, public IntrusivePtrBase<SinglePointRemoveAction> {
  public:
	SinglePointRemoveAction(const SymbolShapeP& shape, UInt position);
	
	virtual String getName(bool to_undo) const { return _("Delete point"); }
	virtual void   perform(bool to_undo);
	
  private:
	SymbolShapeP shape;
	UInt position;
	ControlPointP point;               ///< Removed point
	ControlPointUpdate point1, point2; ///< Points before/after
};

SinglePointRemoveAction::SinglePointRemoveAction(const SymbolShapeP& shape, UInt position)
	: shape(shape)
	, position(position)
	, point (shape->getPoint(position))
	, point1(shape->getPoint(position - 1))
	, point2(shape->getPoint(position + 1))
{
	if (point1.other.segment_after == SEGMENT_CURVE || point2.other.segment_before == SEGMENT_CURVE) {
		// try to preserve curve
		Vector2D before = point->delta_before;
		Vector2D after  = point->delta_after;
		
		// convert both segments to curves first
		if (point1.other.segment_after != SEGMENT_CURVE) {
			before                   = (point1.other.pos - point->pos) / 3.0;
			point1.other.delta_after = -before;
			point1.other.segment_after = SEGMENT_CURVE;
		}
		if (point2.other.segment_before != SEGMENT_CURVE) {
			after                     = (point2.other.pos - point->pos) / 3.0;
			point2.other.delta_before = -after;
			point2.other.segment_before = SEGMENT_CURVE;
		}
		
		// The inverse of adding a point, reconstruct the original handles
		// before being subdivided using de Casteljau's algorithm
		// length of handles
		double bl   = before.length() + 0.00000001; // prevent division by 0
		double al   = after .length() + 0.00000001;
		double totl = bl + al;
		// set new handle sizes
		point1.other.delta_after  *= totl / bl;
		point2.other.delta_before *= totl / al;
		
		// Also take in acount cases where the point does not correspond to a freshly added point.
		// distance from the point to the curve as it would be in the above case can be used,
		// in the case of a point just added this distance = 0
		BezierCurve c(point1.other, point2.other);
		double t = bl / totl;
		Vector2D p = c.pointAt(t);
		Vector2D distP = point->pos - p;
		// adjust handle sizes
		point1.other.delta_after  *= ssqrt(dot(distP, point1.other.delta_after) /point1.other.delta_after.lengthSqr())  + 1;
		point2.other.delta_before *= ssqrt(dot(distP, point2.other.delta_before)/point2.other.delta_before.lengthSqr()) + 1;
	
		// unlock if needed
		if (point1.other.lock == LOCK_SIZE) point1.other.lock = LOCK_DIR;
		if (point2.other.lock == LOCK_SIZE) point2.other.lock = LOCK_DIR;
	} else {
		// just lines, keep it that way
	}
}

void SinglePointRemoveAction::perform(bool to_undo) {
	if (to_undo) {
		// reinsert the point
		shape->points.insert(shape->points.begin() + position, point);
	} else {
		// remove the point
		shape->points.erase( shape->points.begin() + position);
	}
	// update points around removed point
	point1.perform();
	point2.perform();
}

DECLARE_POINTER_TYPE(SinglePointRemoveAction);
DECLARE_TYPEOF_COLLECTION(SinglePointRemoveActionP);


// Remove a set of points from a symbol shape.
// Internally represented as a list of Single Point Remove Actions.
// Not all points mat be removed, at least two points must remain.
class ControlPointRemoveAction : public Action {
  public:
	ControlPointRemoveAction(const SymbolShapeP& shape, const set<ControlPointP>& to_delete);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	vector<SinglePointRemoveActionP> removals;
};

ControlPointRemoveAction::ControlPointRemoveAction(const SymbolShapeP& shape, const set<ControlPointP>& to_delete) {
	int index = 0;
	// find points to remove, in reverse order
	FOR_EACH(point, shape->points) {
		if (to_delete.find(point) != to_delete.end()) {
			// remove this point
			removals.push_back(new_intrusive2<SinglePointRemoveAction>(shape, index));
		}
		++index;
	}
}

String ControlPointRemoveAction::getName(bool to_undo) const {
	return removals.size() == 1 ? _ACTION_("delete point") : _ACTION_("delete points");
}

void ControlPointRemoveAction::perform(bool to_undo) {
	if (to_undo) {
		FOR_EACH(r, removals) r->perform(to_undo);
	} else {
		// in reverse order, because positions of later points will
		// change after removal of earlier points.
		FOR_EACH_REVERSE(r, removals) r->perform(to_undo);
	}
}


Action* control_point_remove_action(const SymbolShapeP& shape, const set<ControlPointP>& to_delete) {
	if (shape->points.size() - to_delete.size() < 2) {
		// TODO : remove part?
		//new_intrusive<ControlPointRemoveAllAction>(part);
		return 0; // no action
	} else {
		return new ControlPointRemoveAction(shape, to_delete);
	}
}
