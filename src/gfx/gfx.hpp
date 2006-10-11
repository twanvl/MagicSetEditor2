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

#include "../util/prec.hpp"

// ----------------------------------------------------------------------------- : Resampling

/// Resample (resize) an image, uses bilenear filtering
/** The algorithm first resizes in horizontally, then vertically,
 *  the two passes are essentially the same:
 *   - for each row:
 *     - each input pixel becomes a fixed amount of output (in 1<<shift fixed point math)
 *     - for each output pixel:
 *       - 'eat' input pixels until the total is 1<<shift
 *       - write the total to the output pixel
 *   - to ensure the sum of all the pixel amounts is exacly width<<shift an extra rest amount
 *     is 'eaten' from the first pixel
 *
 *  Uses fixed point numbers internally
 */
void resample(const Image& imgIn, Image& imgOut);

/// Resamples an image, first clips the input image to a specified rectangle,
/// that rectangle is resampledinto the entire output image
void resample_and_clip(const Image& imgIn, Image& imgOut, wxRect rect);

// ----------------------------------------------------------------------------- : Image rotation

/// Rotates an image counter clockwise
/// angle must be a multiple of 90, i.e. {0,90,180,270}
Image rotate_image(const Image& image, int angle);

// ----------------------------------------------------------------------------- : Blending

/// Blends two images together, using a horizontal gradient
/** The result is stored in img1
 *  To the left the color is that of img1, to the right of img2
 */
void hblend(Image& img1, const Image& img2);

/// Blends two images together, using a vertical gradient
void vblend(Image& img1, const Image& img2);

/// Blends two images together, using a third image as a mask
/** The result is stored in img1
 *  mask is used as a mask, white pixels are taken from img1, black pixels from img2
 *  color channels are blended separatly
 */
void mask_blend(Image& img1, const Image& img2, const Image& mask);

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

// ----------------------------------------------------------------------------- : Utility

inline int bot(int x) { return max(0,   x); } ///< bottom range check for color values
inline int top(int x) { return min(255, x); } ///< top    range check for color values
inline int col(int x) { return top(bot(x)); } ///< top and bottom range check for color values

// ----------------------------------------------------------------------------- : EOF
#endif
