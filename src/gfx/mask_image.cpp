//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
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
	delete[] alpha;
}

void AlphaMask::setAlpha(Image& img) const {
	set_alpha(img, alpha, size);
}

void AlphaMask::setAlpha(Bitmap& bmp) const {
	Image img = bmp.ConvertToImage();
	setAlpha(img);
	bmp = Bitmap(img);
}

bool AlphaMask::isTransparent(int x, int y) const {
	if (x < 0 || y > 0 || x >= size.x || y >= size.y) return false;
	return alpha[x + y * size.x] < 20;
}

bool convex(const wxPoint& p, const wxPoint& q, const wxPoint& r) {
	return p.y*q.x - p.x*q.y - p.y*r.x + q.y*r.x + p.x*r.y - q.x*r.y > 0;
}
void make_convex(vector<wxPoint>& points) {
	while (points.size() > 2 &&
	       !convex(points[points.size() - 3]
	              ,points[points.size() - 2]
	              ,points[points.size() - 1])) {
		points.erase(points.end() - 2);
	}
}

void AlphaMask::convexHull(vector<wxPoint>& points) const {
	// Left side
	int miny = size.y, maxy = -1, lastx = 0;
	for (int y = 0 ; y < size.y ; ++y) {
		for (int x = 0 ; x < size.x ; ++x) {
			if (alpha[x + y * size.x] >= 20) {
				// opaque pixel
				miny = min(miny,y);
				maxy = y;
				if (y == miny) {
					points.push_back(wxPoint(x-1,y-1));
				}
				points.push_back(wxPoint(x-1,y));
				make_convex(points);
				lastx = x;
				break;
			}
		}
	}
	if (maxy == -1) return; // No image
	points.push_back(wxPoint(lastx-1,maxy+1));
	make_convex(points);
	// Right side
	for (int y = maxy ; y >= miny ; --y) {
		for (int x = size.x - 1 ; x >= 0 ; --x) {
			if (alpha[x + y * size.x] >= 20) {
				// opaque pixel
				if (y == maxy) {
					points.push_back(wxPoint(x+1,y+1));
					make_convex(points);
				}
				points.push_back(wxPoint(x+1,y));
				make_convex(points);
				lastx = x;
				break;
			}
		}
	}
	points.push_back(wxPoint(lastx+1,miny-1));
	make_convex(points);
}

// ----------------------------------------------------------------------------- : ContourMask

ContourMask::ContourMask()
	: width(0), height(0), lefts(nullptr), rights(nullptr)
{}
ContourMask::~ContourMask() {
	unload();
}

void ContourMask::load(const Image& image) {
	unload();
	width  = image.GetWidth();
	height = image.GetHeight();
	lefts  = new int[height];
	rights = new int[height];
	// for each row: determine left and rightmost white pixel
	Byte* data = image.GetData();
	for (int y = 0 ; y < height ; ++y) {
		lefts[y] = width; rights[y] = width;
		for (int x = 0 ; x < width ; ++x) {
			int v = data[0] + data[1] + data[2];
			if (v > 50) { // white enough
				rights[y] = x;
				if (x < lefts[y]) lefts[y] = x;
			}
			data += 3;
		}
	}
}

void ContourMask::unload() {
	delete lefts;
	delete rights;
	lefts = rights = nullptr;
	width = height = 0;
}

double ContourMask::rowLeft (double y, RealSize size) const {
	if (!ok() || y < 0 || y >= size.height) {
		// no mask, or outside it
		return 0;
	}
	return lefts[(int)(y * size.height / height)] * size.width / width;
}
double ContourMask::rowRight(double y, RealSize size) const {
	if (!ok() || y < 0 || y >= size.height) {
		// no mask, or outside it
		return size.width;
	}
	return rights[(int)(y * size.height / height)] * size.width / width;
}
