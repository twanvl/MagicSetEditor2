//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/bezier.hpp>
#include <gfx/polynomial.hpp>

// ----------------------------------------------------------------------------- : Evaluation

BezierCurve::BezierCurve(const Vector2D& p0, const Vector2D& p1, const Vector2D& p2, const Vector2D& p3) {
	// calculate coefficients
	c = (p1 - p0) * 3.0;
    b = (p2 - p1) * 3.0 - c;
    a = (p3 - p0) - c - b;
    d = p0;
}

BezierCurve::BezierCurve(const ControlPoint& p0, const ControlPoint& p3) {
	// calculate coefficients
	c = p0.delta_after * 3.0;
    b = (p3.pos + p3.delta_before - p0.pos - p0.delta_after) * 3.0 - c;
    a = (p3.pos - p0.pos) - c - b;
    d = p0.pos;
}


void deCasteljau(Vector2D a1,  Vector2D  a2, Vector2D  a3, Vector2D  a4,
                 Vector2D& b2, Vector2D& b3, Vector2D& b4c1,
                 Vector2D& c2, Vector2D& c3, double t)
{
	b2             = a1    + (a2    - a1) * t;
	Vector2D mid23 = a2    + (a3    - a2) * t;
	c3             = a3    + (a4    - a3) * t;
	b3             = b2    + (mid23 - b2) * t;
	c2             = mid23 + (c3    - mid23) * t;
	b4c1           = b3    + (c2    - b3) * t;
}

void deCasteljau(ControlPoint& a, ControlPoint& b, double t, ControlPoint& mid) {
	deCasteljau(a.pos, a.delta_after, b.delta_before, b.pos, t, mid);
}

void deCasteljau(const Vector2D& a1, Vector2D& a21, Vector2D& a34, const Vector2D& a4, double t, ControlPoint& out) {
	Vector2D half21   = a21 * t;
	Vector2D half34   = a34 * (1-t);
	Vector2D mid23    = (a1 + a21) * (1-t) + (a34 + a4) * t;
	Vector2D mid23h21 = (a1 + half21) * (1-t) + mid23 * t;
	Vector2D mid23h34 = (a4 + half34) * t     + mid23 * (1-t);
	out.pos         = mid23h21 * (1-t) + mid23h34 * t;
	out.delta_before = mid23h21 - out.pos;
	out.delta_after  = mid23h34 - out.pos;
	a21 = half21;
	a34 = half34;
}

// ----------------------------------------------------------------------------- : Drawing

void curve_subdivide(const BezierCurve& c, const Vector2D& p0, const Vector2D& p1, double t0, double t1, const Vector2D& origin, const Matrix2D& m, vector<wxPoint>& out, UInt level) {
	if (level <= 0)  return;
	double midtime = (t0+t1) * 0.5f;
	Vector2D midpoint = c.pointAt(midtime);
	Vector2D d0 = p0 - midpoint;
	Vector2D d1 = midpoint - p1;
	// Determine treshold for subdivision, greater angle -> subdivide
	// greater size -> subdivide
	double treshold = fabs(  atan2(d0.x,d0.y) - atan2(d1.x,d1.y))  * (p0-p1).lengthSqr();
	bool subdivide = treshold >= .0001;
	// subdivide left
	curve_subdivide(c, p0, midpoint, t0, midtime, origin, m, out, level - 1);
	// add midpoint
	if (subdivide) {
		out.push_back(origin + midpoint * m);
	}
	// subdivide right
	curve_subdivide(c, midpoint, p1, midtime, t1, origin, m, out, level - 1);
}

void segment_subdivide(const ControlPoint& p0, const ControlPoint& p1, const Vector2D& origin, const Matrix2D& m, vector<wxPoint>& out) {
	assert(p0.segment_after == p1.segment_before);
	// always the start
	out.push_back(origin + p0.pos * m);
	if (p0.segment_after == SEGMENT_CURVE) {
		// need more points?
		BezierCurve curve(p0,p1);
		curve_subdivide(curve, p0.pos, p1.pos, 0, 1, origin, m, out, 5);
	}
}

// ----------------------------------------------------------------------------- : Bounds

void segment_bounds(const ControlPoint& p1, const ControlPoint& p2, Vector2D& min, Vector2D& max) {
	assert(p1.segment_after == p2.segment_before);
	if (p1.segment_after == SEGMENT_LINE) {
		line_bounds  (p1.pos, p2.pos, min, max);
	} else {
		bezier_bounds(p1,     p2,     min, max);
	}
}

void bezier_bounds(const ControlPoint& p1, const ControlPoint& p2, Vector2D& min, Vector2D& max) {
	assert(p1.segment_after == SEGMENT_CURVE);
	// First of all, the corners should be in the bounding box
	point_bounds(p1.pos, min, max);
	point_bounds(p2.pos, min, max);
	// Solve the derivative of the bezier curve to find its extremes
	// It's only a quadtratic equation :)
	BezierCurve curve(p1,p2);
	double roots[4];
	UInt count;
	count  = solve_quadratic(3*curve.a.x, 2*curve.b.x, curve.c.x, roots);
	count += solve_quadratic(3*curve.a.y, 2*curve.b.y, curve.c.y, roots + count);
	// now check them for min/max
	for (UInt i = 0 ; i < count ; ++i) {
		double t = roots[i];
		if (t >=0 && t <= 1) {
			point_bounds(curve.pointAt(t), min, max);
		}
	}
}

void line_bounds(const Vector2D& p1, const Vector2D& p2, Vector2D& min, Vector2D& max) {
	point_bounds(p1, min, max);
	point_bounds(p2, min, max);
}

void point_bounds(const Vector2D& p, Vector2D& min, Vector2D& max) {
	min = piecewise_min(min, p);
	max = piecewise_max(max, p);
}

// Is a point inside the bounds <min...max>?
bool point_in_bounds(const Vector2D& p, const Vector2D& min, const Vector2D& max) {
	return p.x >= min.x && p.y >= min.y &&
	       p.x <= max.x && p.y <= max.y;
}


// ----------------------------------------------------------------------------- : Point tests

// Is a point inside a symbol shape?
bool point_in_shape(const Vector2D& pos, const SymbolShape& shape) {
	// Step 1. compare bounding box of the part
	if (!point_in_bounds(pos, shape.min_pos, shape.max_pos)) return false;
	
	// Step 2. trace ray outward, count intersections
	int count = 0;
	size_t size = shape.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		ControlPointP p1 = shape.getPoint((int) i);
		ControlPointP p2 = shape.getPoint((int) i + 1);
		if (p1->segment_after == SEGMENT_LINE) {
			count += intersect_line_ray  (p1->pos, p2->pos, pos);
		} else {
			count += intersect_bezier_ray(*p1,     *p2,     pos);
		}
	}
	
	return count & 1; // odd number of intersections
}


// ----------------------------------------------------------------------------- : Finding points

bool pos_on_segment(const Vector2D& pos, double range, const ControlPoint& p1, const ControlPoint& p2, Vector2D& pOut, double& tOut) {
	if (p1.segment_after == SEGMENT_CURVE) {
		return pos_on_bezier(pos, range, p1,     p2,     pOut, tOut);
	} else {
		return pos_on_line  (pos, range, p1.pos, p2.pos, pOut, tOut);
	}
}

bool pos_on_bezier(const Vector2D& pos, double range, const ControlPoint& p1, const ControlPoint& p2, Vector2D& pOut, double& tOut) {
	assert(p1.segment_after == SEGMENT_CURVE);
	// Find intersections with the horizontal and vertical lines through p0
	// theoretically we would need to check in all directions, but this covers enough
	BezierCurve curve(p1, p2);
	double roots[6];
	UInt count;
	count  = solve_cubic(curve.a.y, curve.b.y, curve.c.y, curve.d.y - pos.y, roots);
	count += solve_cubic(curve.a.x, curve.b.x, curve.c.x, curve.d.x - pos.x, roots + count); // append intersections
	// take the best intersection point
	double bestDistSqr = std::numeric_limits<double>::max(); //infinity
	for(UInt i = 0 ; i < count ; ++i) {
		double t = roots[i];
		if (t >= 0 && t < 1) {
			Vector2D pnt = curve.pointAt(t);
			double distSqr = (pnt - pos).lengthSqr();
			if (distSqr < bestDistSqr) {
				bestDistSqr = distSqr;
				pOut = pnt;
				tOut = t;
			}
		}
	}
	return bestDistSqr <= range * range;
}

bool pos_on_line(const Vector2D& pos, double range, const Vector2D& p1, const Vector2D& p2, Vector2D& pOut, double& t) {
	Vector2D p21 = p2 - p1;
	double p21len = p21.lengthSqr();
	if (p21len < 0.00001) return false; // line is too short
	t = dot(p21, pos - p1) / p21len; // 'time' on line p1->p2
	if (t < 0 || t > 1) return false; // outside segment
	pOut = p1 + p21 * t; // point on line
	Vector2D dist = pOut - pos; // distance to line
	return dist.lengthSqr() <= range * range; // in range?
}

// ----------------------------------------------------------------------------- : Intersection

UInt intersect_bezier_ray(const ControlPoint& p1, const ControlPoint& p2, const Vector2D& pos) {
	// Looking only at the y coordinate
	// we can use the cubic formula to find roots, points where the horizontal line
	// through pos intersects the (extended) curve
	BezierCurve curve(p1,p2);
	double roots[3];
	UInt count = solve_cubic(curve.a.y, curve.b.y, curve.c.y, curve.d.y - pos.y, roots);
	// now check if the solutions are left of pos.x
	UInt solsInRange = 0;
	for(UInt i = 0 ; i < count ; ++i) {
		double t = roots[i];
		if (t >= 0 && t < 1 && curve.pointAt(t).x < pos.x) {
			solsInRange += 1;
		}
	}
	return solsInRange;
}

bool intersect_line_ray(const Vector2D& p1, const Vector2D& p2, const Vector2D& pos) {
	// Vector2D intersection = p1 + t * (p2 - p1)
	// intersection.y == pos.y
	//                == p1.y + t * (p2.y - p1.y)
	// => t == (pos.y - p1.y) / (p2.y - p1.y)
	// intersection.x == p1.x + t * (p2.x - p1.x)
	//                == p1.x + (pos.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y)
	double dy = p2.y - p1.y;
	if (fabs(dy) < 0.0000000001) {
		// horizontal line
		return (p1.x > pos.x || p2.x > pos.x) &&   // starts to the left of pos
		       fabs(p1.y - pos.y) < 0.0000000001;  // same y as pos
	} else {
		double dx = p2.x - p1.x;
		double t  = (pos.y - p1.y) / dy;
		if (t < 0.0 || t >= 1.0) return false;
		double intersectX = p1.x + t * dx;
		return intersectX <= pos.x; // intersection is left of pos
	}
}
