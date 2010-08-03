//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Saturation

void saturate(Image& image, double amount) {
	Byte* pix = image.GetData();
	Byte* end = pix + image.GetWidth() * image.GetHeight() * 3;
	// the formula for saturation is
	//   rgb' = (rgb - amount * avg) / (1 - amount)
	// if amount >= 1 then this is some kind of inversion
	// if amount >  0 then use formula directly
	// if amount <  0 then de-saturate instead:
	//   rgb = rgb' + -amount*avg - -amount*rgb'
	//       = rgb' * (1 - -amount) + -amount*avg
	// if amount < -1 then we are left with just the average
	int factor = int(256 * amount);
	if (factor == 0) {
		return; // nothing to do
	} else if (factor == 256) {
		// super crazy saturation: division by zero
		// if we take infty to be 255, then it is a >avg test
		while (pix != end) {
			int r = pix[0], g = pix[1], b = pix[2];
			pix[0] = r+r > g+b ? 255 : 0;
			pix[1] = g+g > b+r ? 255 : 0;
			pix[2] = b+b > r+g ? 255 : 0;
			pix += 3;
		}
	} else if (factor > 0) {
		int div = 768 - 3 * factor;
		assert(div > 0);
		while (pix != end) {
			int r = pix[0], g = pix[1], b = pix[2];
			int avg = factor*(r+g+b);
			pix[0] = col((768*r - avg) / div);
			pix[1] = col((768*g - avg) / div);
			pix[2] = col((768*b - avg) / div);
			pix += 3;
		}
	} else {
		int factor1 = -factor;
		int factor2 = 768 - 3*factor1;
		while (pix != end) {
			int r = pix[0], g = pix[1], b = pix[2];
			int avg = factor1*(r+g+b);
			pix[0] = (factor2*r + avg) / 768;
			pix[1] = (factor2*g + avg) / 768;
			pix[2] = (factor2*b + avg) / 768;
			pix += 3;
		}
	}
}

// ----------------------------------------------------------------------------- : Color inversion

void invert(Image& img) {
	Byte* data = img.GetData();
	int n = 3 * img.GetWidth() * img.GetHeight();
	for (int i = 0 ; i < n ; ++i) {
		data[i] = 255 - data[i];
	}
}

// ----------------------------------------------------------------------------- : Coloring symbol images

RGB recolor(RGB x, RGB cr, RGB cg, RGB cb, RGB cw) {
	int lo = min(x.r,min(x.g,x.b));
	// amount of each
	int nr = x.r - lo;
	int ng = x.g - lo;
	int nb = x.b - lo;
	int nw = lo;
	// We should have that nr+ng+bw+nw < 255,
	//  otherwise the input is not a mixture of red/green/blue/white.
	// Just to be sure, divide by the sum instead of 255
	int total = max(255, nr+ng+nb+nw);
	
	return RGB(
			static_cast<Byte>( (nr * cr.r + ng * cg.r + nb * cb.r + nw * cw.r) / total ),
			static_cast<Byte>( (nr * cr.g + ng * cg.g + nb * cb.g + nw * cw.g) / total ),
			static_cast<Byte>( (nr * cr.b + ng * cg.b + nb * cb.b + nw * cw.b) / total )
		);
}

void recolor(Image& img, RGB cr, RGB cg, RGB cb, RGB cw) {
	RGB* data = (RGB*)img.GetData();
	int n = img.GetWidth() * img.GetHeight();
	for (int i = 0 ; i < n ; ++i) {
		data[i] = recolor(data[i], cr, cg, cb, cw);
	}
}

Byte to_grayscale(RGB x) {
	return (Byte)((6969 * x.r + 23434 * x.g + 2365 * x.b) / 32768); // from libpng
}

void recolor(Image& img, RGB cr) {
	RGB black(0,0,0), white(255,255,255);
	bool dark = to_grayscale(cr) < 100;
	recolor(img, cr, dark ? black : white, dark ? white : black, white);
}

