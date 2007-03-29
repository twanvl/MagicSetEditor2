//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GFX_GFX
#define HEADER_GFX_GFX

/** @file gfx/gfx.hpp
 *
 *  Graphics/image processing functions.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>

// ----------------------------------------------------------------------------- : Resampling

/// Resample (resize) an image, uses bilenear filtering
void resample(const Image& img_in, Image& img_out);

/// Resamples an image, first clips the input image to a specified rectangle
/** The selected rectangle is resampled into the entire output image */
void resample_and_clip(const Image& img_in, Image& img_out, wxRect rect);

/// How to preserve the aspect ratio of an image when rescaling
enum PreserveAspect
{	ASPECT_STRETCH		///< don't preserve
,	ASPECT_BORDER		///< put borders around the image to make it the right shape
,	ASPECT_FIT			///< generate a smaller image if needed
};

/// Resample an image, but preserve the aspect ratio by adding a transparent border around the output if needed.
void resample_preserve_aspect(const Image& img_in, Image& img_out);

/// Resample an image to create a sharp result by applying a sharpening filter
/** Amount must be between 0 and 100 */
void sharp_resample(const Image& img_in, Image& img_out, int amount);

/// Sharpening version of of resample_and_clip
void sharp_resample_and_clip(const Image& img_in, Image& img_out, wxRect rect, int amount);

/// Draw text by first drawing it using a larger font and then downsampling it
/** optionally rotated by an angle.
 *  rect    = rectangle to draw in
 *  (wc,hc) = the corner where drawing should begin, (0,0) for top-left, (1,1) for bottom-right
 */
void draw_resampled_text(DC& dc, const RealRect& rect, int wc, int hc, int angle, const String& text);

// scaling factor to use when drawing resampled text
extern const int text_scaling;

// ----------------------------------------------------------------------------- : Image rotation

/// Rotates an image counter clockwise
/// angle must be a multiple of 90, i.e. {0,90,180,270}
Image rotate_image(const Image& image, int angle);

// ----------------------------------------------------------------------------- : Blending

/// Blends two images together using some linear gradient
/** The result is stored in img1
 *  The two coordinates give the two points between which the images are blended
 *  Coordinates are given in the range [0..1);
 */
void linear_blend(Image& img1, const Image& img2, double x1,double y1, double x2,double y2);

/// Blends two images together, using a third image as a mask
/** The result is stored in img1
 *  mask is used as a mask, white pixels are taken from img1, black pixels from img2
 *  color channels are blended separatly
 */
void mask_blend(Image& img1, const Image& img2, const Image& mask);

// ----------------------------------------------------------------------------- : Effects

/// Saturate an image, amount should be in range [0...100]
void saturate(Image& image, int amount);

// ----------------------------------------------------------------------------- : Combining

/// Ways in which images can be combined, similair to what Photoshop supports
enum ImageCombine
{	COMBINE_NORMAL
,	COMBINE_ADD
,	COMBINE_SUBTRACT
,	COMBINE_STAMP
,	COMBINE_DIFFERENCE
,	COMBINE_NEGATION
,	COMBINE_MULTIPLY
,	COMBINE_DARKEN
,	COMBINE_LIGHTEN
,	COMBINE_COLOR_DODGE
,	COMBINE_COLOR_BURN
,	COMBINE_SCREEN
,	COMBINE_OVERLAY
,	COMBINE_HARD_LIGHT
,	COMBINE_SOFT_LIGHT
,	COMBINE_REFLECT
,	COMBINE_GLOW
,	COMBINE_FREEZE
,	COMBINE_HEAT
,	COMBINE_AND
,	COMBINE_OR
,	COMBINE_XOR
,	COMBINE_SHADOW
};

/// Combine image b onto image a using some combining function.
/// The results are stored in the image A.
/// This image gets the alpha channel from B, it should then be
/// drawn onto the area where A originated.
void combine_image(Image& a, const Image& b, ImageCombine combine);

/// Draw an image to a DC using a combining function
void draw_combine_image(DC& dc, UInt x, UInt y, const Image& img, ImageCombine combine);

// ----------------------------------------------------------------------------- : Masks

/// Use the red channel of img_alpha as alpha channel for img
void set_alpha(Image& img, const Image& img_alpha);

/// An alpha mask is an alpha channel that can be copied to another image
/** It is created by treating black in the source image as transparent and white (red) as opaque
 */
class AlphaMask {
  public:
	AlphaMask(const Image& mask);
	~AlphaMask();
	
	/// Apply the alpha mask to an image
	void setAlpha(Image& i) const;
	/// Apply the alpha mask to a bitmap
	void setAlpha(Bitmap& b) const;
	
	/// Is the given location fully transparent?
	bool isTransparent(int x, int y) const;
	
	/// Size of the mask
	wxSize size;
  private:
	Byte* alpha;
};

/// A contour mask stores the size and position of each line in the image
/** It is created by treating black in the source image as transparent and white (red) as opaque
 *  The left is the first non-transparent pixel, the right is the last non-transparent pixel
 */
class ContourMask {
  public:
	ContourMask();
	~ContourMask();
	
	/// Load a contour mask
	void load(const String& filename);
	/// Unload the mask
	void unload();
	
	/// Returns the start of a row, when the mask were stretched to size
	double rowLeft (double y, RealSize size) const;
	/// Returns the end of a row, when the mask were stretched to size
	double rowRight(double y, RealSize size) const;
	
  private:
	UInt width, height;
	UInt *lefts, *rights;
};

// ----------------------------------------------------------------------------- : Color utility functions

inline int bot(int x) { return max(0,   x); } ///< bottom range check for color values
inline int top(int x) { return min(255, x); } ///< top    range check for color values
inline int col(int x) { return top(bot(x)); } ///< top and bottom range check for color values

/// Linear interpolation between colors
Color lerp(const Color& a, const Color& b, double t);

/// convert HSL to RGB, h,s,l must be in range [0...1)
Color hsl2rgb(double h, double s, double l);

/// A darker version of a color
Color darken(const Color& c);

/// A saturated version of a color
Color saturate(const Color& c, double amount);

// ----------------------------------------------------------------------------- : EOF
#endif
