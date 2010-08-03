//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GFX_COLOR
#define HEADER_GFX_COLOR

/** @file gfx/color.hpp
 *
 *  Color related functions and types.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Color with alpha

/// Color with alpha channel
class AColor : public Color {
  public:
	Byte alpha;	///< The alpha value, in the range [0..255]
	inline AColor()                                     :               alpha(0) {}
	inline AColor(Byte r, Byte g, Byte b, Byte a = 255) : Color(r,g,b), alpha(a) {}
	inline AColor(const Color& color,     Byte a = 255) : Color(color), alpha(a) {}
	
	inline bool operator == (const AColor& that) const {
		return static_cast<const Color&>(*this) == static_cast<const Color&>(that) && alpha == that.alpha;
	}
	inline bool operator != (const AColor& that) const { return ! (*this == that); }
};

// ----------------------------------------------------------------------------- : Parsing

/// Parse a color
Color parse_color(const String& value);

/// Parse a color with alpha
AColor parse_acolor(const String& value);

/// Convert an AColor to a string
String format_acolor(AColor col);

// ----------------------------------------------------------------------------- : Color utility functions

inline int bot(int x) { return max(0,   x); } ///< bottom range check for color values
inline int top(int x) { return min(255, x); } ///< top    range check for color values
inline int col(int x) { return top(bot(x)); } ///< top and bottom range check for color values

/// Linear interpolation between colors
Color lerp(const Color& a, const Color& b, double t);
/// Linear interpolation between colors
AColor lerp(const AColor& a, const AColor& b, double t);

/// convert HSL to RGB, h,s,l must be in range [0...1)
Color hsl2rgb(double h, double s, double l);

/// A darker version of a color
Color darken(const Color& c);

/// A black or white color, that contrasts with c
Color contrasting_color(const Color& c);

/// A saturated version of a color
Color saturate(const Color& c, double amount);

/// Recolor:
/**
 * Maps
 *   black -> black
 *   red   -> cr
 *   green -> cg
 *   blue  -> cb
 *   white -> cw
 *
 * Allows for interpolation between the colors, so for example
 *   rgb(128,128,128) -> 0.5*cw
 *   rgb(128,128,0)   -> 0.5*cr + 0.5*cg
 *   rgb(128,0,0)     -> 0.5*cr
 */
Color recolor(Color const& c,  Color const& cr, Color const& cg, Color const& cb, Color const& cw);
Image recolor(Image const& im, Color const& cr, Color const& cg, Color const& cb, Color const& cw);
/// Like recolor: map green to similar black/white and blue to complementary white/black
Image recolor(Image const& im, Color const& cr);

/// Fills an image with the specified color
void fill_image(Image& image, const Color& color);

// ----------------------------------------------------------------------------- : EOF
#endif
