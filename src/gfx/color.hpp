//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file gfx/color.hpp
 *
 *  Color related functions and types.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <optional>

// ----------------------------------------------------------------------------- : Colors

// Colors.
// Ideally we would just use wxColour, but that class is stupid because it has an "invalid" state,
// and the default constructor makes invalid values.
struct Color {
  union {
    struct {
      unsigned char r, g, b, a;
    };
    uint32_t packed;
  };

  inline Color() : r(0), g(0), b(0), a(0) {}
  inline Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}
  inline Color(wxColour color) : r(color.Red()), g(color.Green()), b(color.Blue()), a(color.Alpha()) {}

  inline operator wxColour() const { return wxColour(r, g, b, a); }
  inline operator wxPen() const { return wxPen((wxColour)*this); }
  inline operator wxBrush() const { return wxBrush((wxColour)*this); }

  inline bool operator == (Color const& that) const {
    return packed == that.packed;
  }
  inline bool operator != (Color const& that) const {
    return packed != that.packed;
  }

  inline constexpr unsigned char Red() const { return r; }
  inline constexpr unsigned char Green() const { return g; }
  inline constexpr unsigned char Blue() const { return b; }
  inline constexpr unsigned char Alpha() const { return a; }
};

// -----------------------------------------------------------------------------
// RGB Color, packed into 3 bytes
// These are used for arrays in wxImage
// -----------------------------------------------------------------------------

// stupid headers stealing useful names
#undef RGB

// it is important to pack this into 3 bytes, so we can directly convert from wxImage data
#if defined(_MSC_VER)
  #pragma pack(push, 1)
  #define MAKE_PACKED
#else
  #define MAKE_PACKED __attribute__((__packed__))
#endif

/// An RGB triplet, packed into 3 bytes
struct RGB {
  Byte r,g,b; 
  
  RGB() {}
  RGB(Byte x) : r(x), g(x), b(x) {}
  RGB(Byte r, Byte g, Byte b) : r(r), g(g), b(b) {}
  RGB(Color x) : r(x.Red()), g(x.Green()), b(x.Blue()) {}
  RGB(wxColour const& x) : r(x.Red()), g(x.Green()), b(x.Blue()) {}
  
  inline int total() { return r+g+b; }
  
  inline operator wxColour() const {
    return wxColour(r,g,b);
  }
  
  inline bool operator == (RGB const& that) const {
    return r == that.r && g == that.g && b == that.b;
  }
  inline bool operator < (RGB const& that) const {
    if (r < that.r) return true;
    if (r > that.r) return false;
    if (g < that.g) return true;
    if (g > that.g) return false;
    return b < that.b;
  }
  
} MAKE_PACKED;

#ifdef _MSC_VER
  #pragma pack(pop)
#endif

// ----------------------------------------------------------------------------- : Parsing

/// Parse a color
optional<Color> parse_color(const String& value);

/// Convert a Color to a string
String format_color(Color col);

// ----------------------------------------------------------------------------- : Color utility functions

inline int bot(int x) { return max(0,   x); } ///< bottom range check for color values
inline int top(int x) { return min(255, x); } ///< top    range check for color values
inline int col(int x) { return top(bot(x)); } ///< top and bottom range check for color values

/// Linear interpolation between colors
Color lerp(Color a, Color b, double t);

/// convert HSL to RGB, h,s,l must be in range [0...1)
Color hsl2rgb(double h, double s, double l);

/// A darker version of a color
Color darken(Color c);

/// A black or white color, that contrasts with c
Color contrasting_color(Color c);

/// A saturated version of a color
Color saturate(Color c, double amount);

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
RGB recolor(RGB x, RGB cr, RGB cg, RGB cb, RGB cw);
void recolor(Image& img, RGB cr, RGB cg, RGB cb, RGB cw);
/// Like recolor: map green to similar black/white and blue to complementary white/black
void recolor(Image& img, RGB cr);

/// Fills an image with the specified color
void fill_image(Image& image, RGB color);

