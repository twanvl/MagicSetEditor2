//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Saturation

void saturate(Image& image, int amount) {
	if (amount == 0) return; // nothing to do
	int factor = 300 / amount;
	int div    = factor - 2;
	// for each pixel...
	Byte* pix = image.GetData();
	Byte* end = pix + image.GetWidth() * image.GetHeight() * 3;
	while (pix != end) {
		int r = pix[0], g = pix[1], b = pix[2];
		int r2 = (factor * r - g - b) / div;
		int g2 = (factor * g - r - b) / div;
		int b2 = (factor * b - r - g) / div;
		pix[0] = col(r2);
		pix[1] = col(g2);
		pix[2] = col(b2);
		pix += 3;
	}
}
