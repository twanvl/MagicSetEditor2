//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>

// ----------------------------------------------------------------------------- : Implementation

// Rotates an image
// 'Rotater' is a function object that knows how to 'rotate' a pixel coordinate
template <class Rotater>
Image rotate_image_impl(Image img) {
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
struct Rotate90deg {
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

struct Rotate180deg {
	inline static void init(Image& img, UInt w, UInt h) {
		img.Create(w, h, false);
	}
	inline static int offset(UInt x, UInt y, UInt w, UInt h) {
		UInt mx = w - x - 1;
		UInt my = h - y - 1;
		return w * my + mx;
	}
};

struct Rotate270deg {
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

Image rotate_image(const Image& image, Radians angle) {
	double a = constrain_radians(angle);
	if (is_rad0(a))   return image;
	if (is_rad90(a))  return rotate_image_impl<Rotate90deg> (image);
	if (is_rad180(a)) return rotate_image_impl<Rotate180deg>(image);
	if (is_rad270(a)) return rotate_image_impl<Rotate270deg>(image);
	else {
		if (!image.HasAlpha()) const_cast<Image&>(image).InitAlpha();
		return image.Rotate(angle, wxPoint(0,0));
	}
}


// ----------------------------------------------------------------------------- : Flipping images

// reverse a list of n chunks of size 'step'
void do_flip(Byte const* in, Byte* out, int step, int n) {
	for (int i = 0, j = n-1 ; i < n ; ++i, --j) {
		memcpy(&out[i*step], &in[j*step], step);
	}
}
void do_flip(Byte const* in, Byte* out, int step1, int n1, int n2) {
	int step2 = step1 * n1;
	for (int i = 0 ; i < n2 ; ++i) {
		do_flip(in,out,step1,n1);
		in  += step2;
		out += step2;
	}
}

Image flip_image_horizontal(Image const& img) {
	int w = img.GetWidth(), h= img.GetHeight();
	Image out(w,h,false);
	do_flip(img.GetData(), out.GetData(), 3, w, h);
	if (img.HasAlpha()) {
		out.InitAlpha();
		do_flip(img.GetAlpha(), out.GetAlpha(), 1, w, h);
	}
	return out;
}

Image flip_image_vertical(Image const& img) {
	int w = img.GetWidth(), h= img.GetHeight();
	Image out(w,h,false);
	do_flip(img.GetData(), out.GetData(), 3 * w, h);
	if (img.HasAlpha()) {
		out.InitAlpha();
		do_flip(img.GetAlpha(), out.GetAlpha(), 1 * w, h);
	}
	return out;
}
