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
	
	/// Addition of two sizes
	inline void operator += (const RealSize& s2) {
		width  += s2.width;
		height += s2.height;
	}
	/// Addition of two sizes
	inline RealSize operator + (const RealSize& s2) const {
		return RealSize(width + s2.width, height + s2.height);
	}
	/// Difference of two sizes
	inline void operator -= (const RealSize& s2){
		width  -= s2.width;
		height -= s2.height;
	}
	/// Difference of two sizes
	inline RealSize operator - (const RealSize& s2) const {
		return RealSize(width - s2.width, height - s2.height);
	}
	/// Inversion of a size, inverts both components
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
		return wxSize(realRound(width), realRound(height));
	}
};

// ----------------------------------------------------------------------------- : Rectangle using doubles

/// A rectangle (postion and size) using real (double) coordinats
class RealRect {
  public:
	/// Position of the top left corner
	RealPoint position;
	/// Size of the rectangle
	RealSize  size;
	
	inline RealRect(const RealPoint& position, const RealSize& size)
		: position(position), size(size)
	{}
	inline RealRect(double x, double y, double w, double h)
		: position(x,y), size(w,h)
	{}
};

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
