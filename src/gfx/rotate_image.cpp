//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "../util/prec.hpp"
#include "gfx.hpp"

// ----------------------------------------------------------------------------- : Implementation

// Rotates an image
// 'Rotater' is a function object that knows how to 'rotate' a pixel coordinate
template <class Rotater>
Image rotateImageImpl(Image img) {
	UInt width = img.GetWidth(), height = img.GetHeight();
	// initialize the return image
	Image ret;
	Rotater::init(ret, width, height);
	Byte* in = img.GetData(), *out = ret.GetData();
	// rotate each pixel
	for (UInt y = 0 ; y < height ; ++y) {
		for (UInt x = 0 ; x < width ; ++x) {
			memcpy(out + 3 * Rotater::offset(x, y, width, height), in, 3);
			in += 3;
		}
	}
	// don't forget alpha
	if (img.HasAlpha()) {
		ret.InitAlpha();
		in  = img.GetAlpha();
		out = ret.GetAlpha();
		for (UInt y = 0 ; y < height ; ++y) {
			for (UInt x = 0 ; x < width ; ++x) {
				out[Rotater::offset(x, y, width, height)] = *in;
				in += 1;
			}
		}
	}
	// ret is rotated image
	return ret;
}

// ----------------------------------------------------------------------------- : Rotations

// Function object to handle rotation
struct Rotate90 {
	/// Init a rotated image, where the source is w * h pixels
	inline static void init(Image& img, UInt w, UInt h) {
		img.Create(h, w, false);
	}
	/// Offset in the target data, x, y, w, h are SOURCE coordintes
	inline static int offset(UInt x, UInt y, UInt w, UInt h) {
		int mx = y;
		int my = w - x - 1;
		return h * my + mx; // note: h, since that is the width of the target image
	}
};

struct Rotate180 {
	inline static void init(Image& img, UInt w, UInt h) {
		img.Create(w, h, false);
	}
	inline static int offset(UInt x, UInt y, UInt w, UInt h) {
		UInt mx = w - x - 1;
		UInt my = h - y - 1;
		return w * my + mx;
	}
};

struct Rotate270 {
	inline static void init(Image& img, UInt w, UInt h) {
		img.Create(h, w, false);
	}
	inline static int offset(UInt x, UInt y, UInt w, UInt h) {
		UInt mx = h - y - 1;
		UInt my = x;
		return h * my + mx;
	}
};

// ----------------------------------------------------------------------------- : Interface

Image rotateImageBy(const Image& image, int angle) {
	if (angle == 90) {
		return rotateImageImpl<Rotate90>(image);
	} else if (angle == 180){
		return rotateImageImpl<Rotate180>(image);
	} else if (angle == 270){
		return rotateImageImpl<Rotate270>(image);
	} else{
		return image;
	}
}
