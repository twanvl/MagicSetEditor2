//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_REAL_POINT
#define HEADER_UTIL_REAL_POINT

/** @file util/real_point.hpp
 *
 *  @brief Points and sizes with floating point (real) coordinates.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vector2d.hpp>

// ----------------------------------------------------------------------------- : Point using doubles

/// A point using real (double) coordinates
typedef Vector2D RealPoint;

// ----------------------------------------------------------------------------- : Size using doubles

/// A size (width,height) using real (double) coordinates
class RealSize {
  public:
	double width;
	double height;
	
	inline RealSize()
		: width(0), height(0)
	{}
	inline RealSize(double w, double h)
		: width(w), height(h)
	{}
	inline RealSize(wxSize s)
		: width(s.GetWidth()), height(s.GetHeight())
	{}
	
	/// Negation of a size, negates both components
	inline RealSize operator - () const {
		return RealSize(-width, -height);
	}
	
	/// Multiplying a size by a scalar r, multiplies both components
	inline void operator *= (double r) {
		width  *= r;
		height *= r;
	}
	/// Multiplying a size by a scalar r, multiplies both components
	inline RealSize operator * (double r) const {
		return RealSize(width * r, height * r);
	}
	/// Dividing a size by a scalar r, divides both components
	inline RealSize operator / (double r) const {
		return RealSize(width / r, height / r);
	}
	
	/// Can be converted to a wxSize, with integer components
	inline operator wxSize() {
		return wxSize(to_int(width), to_int(height));
	}
};

/// Add two sizes horizontally
/**  ####   $$$    ####$$$
 *   #### + $$$  = ####$$$
 *   ####          ####...
 */
inline RealSize addHorizontal(const RealSize& a, const RealSize& b) {
	return RealSize(a.width + b.width, max(a.height, b.height));
}

/// Add two sizes vertically
/**  ####   $$$    ####
 *   #### + $$$  = ####
 *   ####          ####
 *                 $$$.
 *                 $$$.
 */
inline RealSize addVertical(const RealSize& a, const RealSize& b) {
	return RealSize(max(a.width, b.width), a.height + b.height);
}

/// Add two sizes diagonally
/**  ####   $$$    ####...
 *   #### + $$$  = ####...
 *   ####          ####...
 *                 ....$$$
 *                 ....$$$
 */
inline RealSize addDiagonal(const RealSize& a, const RealSize& b) {
	return RealSize(a.width + b.width, a.height + b.height);
}

// ----------------------------------------------------------------------------- : Rectangle using doubles

/// A rectangle (postion and size) using real (double) coordinats
class RealRect : private RealPoint, private RealSize {
  public:
	using RealPoint::x;
	using RealPoint::y;
	using RealSize::width;
	using RealSize::height;
		
	inline RealRect(const wxRect& rect)
		: RealPoint(rect.x, rect.y), RealSize(rect.width, rect.height)
	{}
	inline RealRect(const RealPoint& position, const RealSize& size)
		: RealPoint(position), RealSize(size)
	{}
	inline RealRect(double x, double y, double w, double h)
		: RealPoint(x,y), RealSize(w,h)
	{}
	
	/// Position of the top left corner
	inline       RealPoint& position()       { return *this; }
	inline const RealPoint& position() const { return *this; }
	/// Size of the rectangle
	inline       RealSize&  size()           { return *this; }
	inline const RealSize&  size()     const { return *this; }
	
	inline double left()   const { return x; }
	inline double right()  const { return x + width; }
	inline double top()    const { return y; }
	inline double bottom() const { return y + height; }
	
	inline RealPoint topLeft    () const { return *this; }
	inline RealPoint topRight   () const { return RealPoint(x + width, y); }
	inline RealPoint bottomLeft () const { return RealPoint(x,         y + height); }
	inline RealPoint bottomRight() const { return RealPoint(x + width, y + height); }
	
	/// Return a rectangle that is amount larger to all sides
	inline RealRect grow(double amount) {
		return RealRect(x - amount, y - amount, width + 2 * amount, height + 2 * amount);
	}
	/// Move the coordinates by some amount
	inline RealRect move(double dx, double dy, double dw, double dh) const {
		return RealRect(x + dx, y + dy, width + dw, height + dh);
	}
	
	inline operator wxRect() const {
		// Prevent rounding errors, for example if
		// x = 0.6 and width = 0.6
		// the right = 1.2
		// so we want a rectangle from 0 to 1
		// not from 0 to 0
		int i_l = to_int(x), i_r = to_int(right());
		int i_t = to_int(y), i_b = to_int(bottom());
		return wxRect(i_l, i_t, i_r - i_l, i_b - i_t);
	}
};

/// Split a rectangle horizontally
/** Returns a section from the left of this rectangle witht the given width
 *  The rectangle will change to become the part remaining to the right
 *  For example given a rectangle:
 *    +------------+
 *    |     R      |
 *    +------------+
 *  A = split_left(R,5)
 *    +----+-------+
 *    | A  |   R   |
 *    +----+-------+
 */
inline RealRect split_left(RealRect& r, double w) {
	RealRect result(r.x, r.y, w, r.height);
	r.width -= w;
	r.x     += w;
	return result;
}
/// Split a rectangle horizontally
inline RealRect split_left(RealRect& r, const RealSize& s) {
	return split_left(r, s.width);
}


// ----------------------------------------------------------------------------- : Operators

inline RealPoint operator + (const RealSize& s, const RealPoint& p) {
	return RealPoint(p.x + s.width, p.y + s.height);
}
inline RealPoint operator + (const RealPoint& p, const RealSize& s) {
	return RealPoint(p.x + s.width, p.y + s.height);
}
inline RealPoint operator - (const RealPoint& p, const RealSize& s) {
	return RealPoint(p.x - s.width, p.y - s.height);
}
inline void operator += (RealPoint& p, const RealSize& s) {
	p.x += s.width;
	p.y += s.height;
}

// ----------------------------------------------------------------------------- : EOF
#endif
