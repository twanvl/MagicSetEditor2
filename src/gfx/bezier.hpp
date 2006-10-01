//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GFX_BEZIER
#define HEADER_GFX_BEZIER

/** @file gfx/bezier.hpp
 *
 *  Bezier curve and line related functions
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <data/symbol.hpp>

// ----------------------------------------------------------------------------- : Evaluation

/// A bezier curve for evaluation
class BezierCurve {
  public:
	/// coefficients of the equation (x,y) = at^3 + bt^2 + ct + d
	Vector2D a, b, c, d;
	
	/// Construct a bezier curve evaluator given the 4 handles
	BezierCurve(const Vector2D& p0, const Vector2D& p1, const Vector2D& p2, const Vector2D& p3);
	
	/// Construct a bezier curve evaluator given two ControlPoints at the ends
	BezierCurve(const ControlPoint& p0, const ControlPoint& p3);
	
	/// Return the point on this curve at time t in [0...1)
	inline Vector2D pointAt(double t) const {
		return d + (c + (b + a * t) * t) * t;
	}
	
	/// Return the tangent on this curve at time t in [0...1)	
	inline Vector2D tangentAt(double t) const {
		return c + ((b * 2) + (a * 3) * t) * t;
	}
};

/// Subdivide a curve from a to b, store the result in a control point
/** Also modifies the handles of the points to accomodate the inserted point
 *  Direct version, using input curve a1,a2,a3,a4 and output curves a1,b2,b3,b4 and c1,c2,c3,a4
 */
void deCasteljau(Vector2D a1,  Vector2D  a2, Vector2D  a3, Vector2D  a4,
                 Vector2D& b2, Vector2D& b3, Vector2D& b4c1,
                 Vector2D& c2, Vector2D& c3, double t);

/// Subdivide a curve from a to b at time t
/** Stores the point at time t in mid, updates the handles of a and b!
 */
void deCasteljau(ControlPoint& a, ControlPoint& b, double t, ControlPoint& mid);

/// Subdivide a curve from a to b, store the result in a control point
/** Also modifies the handles of the points to accomodate the inserted point!
 */
void deCasteljau(const Vector2D& a1, Vector2D& a21, Vector2D& a34, const Vector2D& a4, double t, ControlPoint& out);

// ----------------------------------------------------------------------------- : Drawing

/// Devide a segment into a number of straight lines for display purposes
/** Adds the resulting corner points of those lines to out, the last point is not added.
 *  All points are converted to display coordinates using rot.tr
 */
void segmentSubdivide(const ControlPoint& p0, const ControlPoint& p1, const Rotation& rot, vector<wxPoint>& out);

// ----------------------------------------------------------------------------- : Bounds

/// Find a bounding box that fits a segment (either a line or a bezier curve) between p1 and p2.
/** stores the results in min and max.
 *  min is only changed if the minimum is smaller then the current value in min,
 *  max only if the maximum is larger.
 */
void segmentBounds(const ControlPoint& p1, const ControlPoint& p2, Vector2D& min, Vector2D& max);

/// Find a bounding box that fits a curve between p1 and p2, stores the results in min and max.
/** min is only changed if the minimum is smaller then the current value in min,
 *  max only if the maximum is larger
 */
void bezierBounds(const ControlPoint& p1, const ControlPoint& p2, Vector2D& min, Vector2D& max);

/// Find a bounding box that fits around p1 and p2, stores the result in min and max
/** min is only changed if the minimum is smaller then the current value in min,
 *  max only if the maximum is larger
 */
void lineBounds(const Vector2D& p1, const Vector2D& p2, Vector2D& min, Vector2D& max);

/// Find a bounding 'box' that fits around a single point
/** min is only changed if the minimum is smaller then the current value in min,
 *  max only if the maximum is larger
 */
void pointBounds(const Vector2D& p, Vector2D& min, Vector2D& max);

// ----------------------------------------------------------------------------- : Point tests

/// Is a point inside the given symbol part?
bool pointInPart(const Vector2D& p, const SymbolPart& part);

// ----------------------------------------------------------------------------- : Finding points

/// Finds the position of p0 on the line between p1 and p2, returns true if the point is on (or near) the line
/// the line between p1 and p2 can also be a bezier curve
/** Returns the time on the segment in tOut, and the point on the segment in pOut
 */
bool posOnSegment(const Vector2D& pos, double range, const ControlPoint& p1, const ControlPoint& p2, Vector2D& pOut, double& tOut);

/// Finds the position of p0 on the line between p1 and p2, returns true if the point is on (or near)
/// the bezier curve between p1 and p2
bool posOnBezier (const Vector2D& pos, double range, const ControlPoint& p1, const ControlPoint& p2, Vector2D& pOut, double& tOut);

/// Finds the position of p0 on the line p1-p2, returns true if the point is withing range of the line
/// if that is the case then (x,y) = p1 + (p2-p1) * out
bool posOnLine   (const Vector2D& pos, double range, const Vector2D& p1,     const Vector2D& p2,     Vector2D& pOut, double& tOut);

// ----------------------------------------------------------------------------- : Intersection

/// Counts the number of intersections between the ray/halfline from (-inf, pos.y) to pos
/// and the bezier curve between p1 and p2.
UInt intersectBezierRay(const ControlPoint& p1, const ControlPoint& p2, const Vector2D& pos);

// Does the line between p1 and p2 intersect the ray (half line) from (-inf, pos.y) to pos?
bool intersectLineRay(const Vector2D& p1, const Vector2D& p2, const Vector2D& pos);

// ----------------------------------------------------------------------------- : EOF
#endif
