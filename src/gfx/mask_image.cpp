//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : AlphaMask

AlphaMask::AlphaMask(const Image& img)
	: size(img.GetWidth(), img.GetHeight())
{
	// Copy red chanel to alpha
	size_t n = size.GetWidth() * size.GetHeight();
	alpha = new Byte[n];
	Byte* from = img.GetData(), *to = alpha;
	for (size_t i = 0 ; i < n ; ++i) {
		*to = *from;
		from += 3;
		to   += 1;
	}
}

AlphaMask::~AlphaMask() {
	delete alpha;
}

void AlphaMask::setAlpha(Image& img) const {
	if (img.GetWidth() != size.GetWidth() || img.GetHeight() != size.GetHeight()) {
		throw InternalError(_("Image used with maks must have same size as mask"));
	}
	if (!img.HasAlpha()) img.InitAlpha();
	memcpy(img.GetAlpha(), alpha, size.GetWidth() * size.GetHeight());
}

void AlphaMask::setAlpha(Bitmap& bmp) const {
	Image img = bmp.ConvertToImage();
	setAlpha(img);
	bmp = Bitmap(img);
}

bool AlphaMask::isTransparent(int x, int y) const {
	assert(x >= 0 && y >= 0 && x < size.GetWidth() && y < size.GetHeight());
	return alpha[x + y * size.GetWidth()] < 20;
}

// ----------------------------------------------------------------------------- : ContourMask
