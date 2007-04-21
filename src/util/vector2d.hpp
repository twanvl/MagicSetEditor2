//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_VECTOR2D
#define HEADER_UTIL_VECTOR2D

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <limits>

// ----------------------------------------------------------------------------- : Rounding

/// Rounding function for converting doubles to integers,
/** Intentionally uses slightly less then 0.5, to give a more consistent result
 *  when for instance something like "x/2" is used. */
inline int to_int(double d) {
	return static_cast<int>(d > 0 ? d + 0.4999995 : d - 0.4999995);
}

// ----------------------------------------------------------------------------- : Vector2D

/// A simple 2d vector class
class Vector2D {
  public:
	/// Coordinates of this vector
	double x, y;
	
	/// Default contructor
	inline Vector2D()
		: x(0), y(0) {}
	/// Contructor with given x and y values
	inline Vector2D(double x, double y)
		: x(x), y(y) {}
	
	/// Addition of two vectors
	inline Vector2D operator + (Vector2D p2) const {
		return Vector2D(x + p2.x, y + p2.y);
	}
	/// Addition of two vectors
	inline void     operator += (Vector2D p2) {
		x += p2.x;  y += p2.y;
	}
	/// Subtract two vectors
	inline Vector2D operator - (Vector2D p2) const {
		return Vector2D(x - p2.x, y - p2.y);
	}
	/// Subtract two vectors
	inline void     operator -= (Vector2D p2) {
		x -= p2.x;  y -= p2.y;
	}
	/// Invert a vector
	inline Vector2D operator - () const {
		return Vector2D(-x, -y);
	}
	
	/// Multiply with a scalar
	inline Vector2D operator * (double r) const {
		return Vector2D(x * r, y * r);
	}
	/// Multiply with a scalar
	inline void     operator *= (double r) {
		x *= r;  y *= r;
	}
	/// Divide by a scalar
	inline Vector2D operator / (double r) const {
		return Vector2D(x / r, y / r);
	}
	/// Divide by a scalar
	inline void     operator /= (double r) {
		x /= r;  y /= r;
	}
	
	/// Inner product with another vector
	inline double dot(Vector2D p2) const {
		return (x * p2.x) + (y * p2.y);
	}
	/// Outer product with another vector
	inline double cross(Vector2D p2) const {
		return (x * p2.y) - (y * p2.x);
	}
	
	/// Piecewise multiplication
	inline Vector2D mul(Vector2D p2) {
		return Vector2D(x * p2.x, y * p2.y);
	}
	/// Piecewise division
	inline Vector2D div(Vector2D p2) {
		return Vector2D(x / p2.x, y / p2.y);
	}
	
	/// Apply a 'matrix' to this vector
	inline Vector2D mul(Vector2D mx, Vector2D my) {
		return Vector2D(dot(mx), dot(my));
	}
	
	/// Returns the square of the length of this vector
	inline double lengthSqr() const {
		return x*x + y*y;
	}
	/// Returns the length of this vector
	inline double length() const {
		return sqrt(lengthSqr());
	}
	/// Returns a normalized version of this vector
	/// i.e. length() == 1
	inline Vector2D normalized() const {
		return *this / length();
	}
	
	inline operator wxPoint() const {
		return wxPoint(to_int(x), to_int(y));
	}
	
	// Vector at infinity
	static inline Vector2D infinity() {
		double inf = numeric_limits<double>::infinity();
		return Vector2D(inf, inf);
	}
};

/// Piecewise minimum
inline Vector2D piecewise_min(const Vector2D& a, const Vector2D& b) {
	return Vector2D(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y
	);
}

/// Piecewise maximum
inline Vector2D piecewise_max(const Vector2D& a, const Vector2D& b) {
	return Vector2D(
		a.x < b.x ? b.x : a.x,
		a.y < b.y ? b.y : a.y
	);
}

inline Vector2D operator * (double a, const Vector2D& b) { return b * a; }


// ----------------------------------------------------------------------------- : EOF
#endif
