//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Saturation

void saturate(Image& image, double amount) {
	if (amount == 0) return; // nothing to do
	Byte* pix = image.GetData();
	Byte* end = pix + image.GetWidth() * image.GetHeight() * 3;
	if (amount > 0) {
		amount = min(amount,0.99);
		int factor = int(256 * amount);
		int div    = 768 - 3 * factor;
		while (pix != end) {
			int r = pix[0], g = pix[1], b = pix[2];
			int avg = factor*(r+g+b);
			pix[0] = col((768*r - avg) / div);
			pix[1] = col((768*g - avg) / div);
			pix[2] = col((768*b - avg) / div);
			pix += 3;
		}
	} else if (amount < -0.99) {
		while (pix != end) {
			int r = pix[0], g = pix[1], b = pix[2];
			pix[0] = pix[1] = pix[2] = (r+g+b)/3;
			pix += 3;
		}
	} else {
		int factor1 = int(256 * -amount);
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
