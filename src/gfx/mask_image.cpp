//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : AlphaMask

AlphaMask::AlphaMask()                 : alpha(nullptr), lefts(nullptr), rights(nullptr) {}
AlphaMask::AlphaMask(const Image& img) : alpha(nullptr), lefts(nullptr), rights(nullptr) {
	load(img);
}
AlphaMask::~AlphaMask() {
	delete[] alpha;
	delete[] lefts;
	delete[] rights;
}

void AlphaMask::load(const Image& img) {
	size_t old_n = alpha ? size.x * size.y : 0;
	size.x = img.GetWidth();
	size.y = img.GetHeight();
	// Memory
	size_t n = size.x * size.y;
	if (n != old_n) {
		delete[] alpha;
		alpha = new Byte[n];
	}
	delete[] lefts;  lefts  = nullptr;
	delete[] rights; rights = nullptr;
	// Copy red chanel to alpha
	Byte* from = img.GetData(), *to = alpha;
	for (size_t i = 0 ; i < n ; ++i) {
		to[i] = from[3*i];
	}
}


void AlphaMask::setAlpha(Image& img) const {
	if (!alpha) return;
	set_alpha(img, alpha, size);
}

void AlphaMask::setAlpha(Bitmap& bmp) const {
	if (!alpha) return;
	Image img = bmp.ConvertToImage();
	setAlpha(img);
	bmp = Bitmap(img);
}

bool AlphaMask::isTransparent(int x, int y) const {
	if (x < 0 || y < 0 || x >= size.x || y >= size.y) return false;
	if (!alpha) return true;
	return alpha[x + y * size.x] < 20;
}

/// Do the points form a (counter??)clockwise angle?
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
	if (!alpha) throw InternalError(_("AlphaMask::convexHull"));
	// Left side, top to bottom
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
	// Right side, bottom to top
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

Image AlphaMask::colorImage(const Color& color) const {
	Image image(size.x, size.y);
	fill_image(image, color);
	setAlpha(image);
	return image;
}

// ----------------------------------------------------------------------------- : Contour Mask

void AlphaMask::loadRowSizes() const {
	if (lefts || !alpha) return;
	lefts  = new int[size.y];
	rights = new int[size.y];
	// for each row: determine left and rightmost white pixel
	for (int y = 0 ; y < size.y ; ++y) {
		lefts[y]  = size.x;
		rights[y] = 0;
		for (int x = 0 ; x < size.x ; ++x) {
			if (alpha[y * size.x + x] > 64) { // white enough
				rights[y] = x;
				if (x < lefts[y]) lefts[y] = x;
			}
		}
	}
}

double AlphaMask::rowLeft (double y, RealSize resize) const {
	loadRowSizes();
	if (!lefts || y < 0 || y >= resize.height) {
		// no mask, or outside it
		return 0;
	}
	return lefts[(int)(y * resize.height / size.y)] * resize.width / size.x;
}

double AlphaMask::rowRight(double y, RealSize resize) const {
	loadRowSizes();
	if (!rights || y < 0 || y >= resize.height) {
		// no mask, or outside it
		return resize.width;
	}
	return rights[(int)(y * resize.height / size.y)] * resize.width / size.x;
}
