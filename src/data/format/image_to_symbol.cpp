//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/image_to_symbol.hpp>
#include <gfx/bezier.hpp>
#include <util/error.hpp>
#include <util/platform.hpp>

DECLARE_TYPEOF_COLLECTION(ControlPointP);
DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Image preprocessing

enum ImageMarker
{	EMPTY  = 0	// This cell is empty
,	FULL   = 1	// This cell is full
,	MARKED = 2	// This cell is full, but it has been used as a starting point for finding symbols
};


/// Convert an image to greyscale
/** The image becomes a single channel image, just an array of bytes.
 *  This means only the first 1/3 of the image data is used, and the image
 *  is no longer an actual image.
 */
void greyscale(Image& img) {
	UInt size = img.GetWidth() * img.GetHeight();
	Byte* data = img.GetData();
	Byte* out  = data;
	for (UInt i = 0 ; i < size ; ++i) {
		*out++ = (data[0] + data[1] + data[2]) / 3;
		data += 3;
	}
}

/// Thresholds an image, giving a black & white result
/** The threshold is determined automatically
 *  The output is stored in the data array,
 *  EMPTY for the 'border' color, FULL for the interior
 */
void threshold(Byte* data, int w, int h) {
	size_t size = w * h;
	// make histogram of data
	size_t hist[256];
	fill_n(hist,256,0);
	for (size_t i = 0 ; i < size ; ++i) {
		hist[data[i]]++;
	}
	// find threshold
	size_t threshold_pos = size / 2;
	int threshold = 255;
	size_t below = 0;
	for (int i = 0 ; i < 255 ; ++i) {
		if (below + hist[i]/2 > threshold_pos) {
			threshold = i;
			break;
		}
		below += hist[i];
		if (below >= threshold_pos) {
			threshold = i + 1;
			break;
		}
	}
	// threshold data
	for (size_t i = 0 ; i < size ; ++i) {
		data[i] = data[i] >= threshold ? FULL : EMPTY;
	}
	// should the colors be inverted?
	int border_count = 0;
	for (int x = 0 ; x < w ; ++x) {
		border_count += data[x] + data[x+(h-1)*w];
	}
	for (int y = 0 ; y < h ; ++y) {
		border_count += data[y*w] + data[w-1+y*w];
	}
	if (border_count > w + h) {
		// more then half the border if FULL, invert
		for (size_t i = 0 ; i < size ; ++i) {
			data[i] = data[i] == FULL ? EMPTY : FULL;
		}
	}
}


// ----------------------------------------------------------------------------- : Image to symbol

bool is_mse1_symbol(const Image& img) {
	// mse1 symbols are 60x80
	if (img.GetWidth() != 60 || img.GetHeight() != 80) return false;
	// the right side is black & white
	int delta = 0;
	for (int y = 0 ; y < 80 ; ++y) {
		Byte* d = img.GetData() + 3 * (y * 60 + 20);
		for (int x = 20 ; x < 60 ; ++x) {
			int r = *d++;
			int g = *d++;
			int b = *d++;
			delta += abs(r - b) + abs(r - g) + abs(b - g);
		}
	}
	if (delta > 5000) return false; // not black & white enough
	// TODO : more checks?
	return true;
}

struct ImageData {
	int width, height;
	Byte* data;
	mutable Byte dummy;
	inline Byte& operator () (int x, int y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) {
			return (dummy = EMPTY); // outside, return empty
		} else {
			return data[x + y*width];
		}
	}
};

bool find_symbol_shape_start(const ImageData& data, int& x_out, int& y_out) {
	for (int x = 0 ; x < data.width ; ++x) {
		for (int y = 0 ; y < data.height ; ++y) {
			if (data(x, y) == FULL && data(x, y-1) == EMPTY) {
				// the point above must be clear, we don't want to start in the 'ground'
				// also, we don't want to find things we found before
				x_out = x;
				y_out = y;
				return true;
			}
		}
	}
	return false;
}

SymbolShapeP read_symbol_shape(const ImageData& data) {
	// find start point
	int xs, ys;
	if (!find_symbol_shape_start(data, xs, ys))  return SymbolShapeP();
	data(xs, ys) |= MARKED;
	
	SymbolShapeP shape(new SymbolShape);
	
	// walk around, clockwise
	xs += 1; // start right of the found point, otherwise last_move might think we came from above
	int x = xs, y = ys;
	int old_x = x, old_y = y;
	int last_move = 1; // 1 = right or down, (as in x|y += 1)
	do {
		// the cursor (x,y) is between four pounts:
		// a b
		//  .
		// c d
		bool a = data(x-1, y-1) & FULL;
		bool b = data(x,   y-1) & FULL;
		bool c = data(x-1, y  ) & FULL;
		bool d = data(x,   y  ) & FULL;
		UInt pack = (a << 12) + (b << 8) + (c << 4) + d; // 0xabcd
		switch (pack) {
			case 0x0001 : x += 1; break;
			case 0x0010 : y += 1; break;
			case 0x0011 : x += 1; break;
			case 0x0100 : y -= 1; break;
			case 0x0101 : y -= 1; break;
			case 0x0110 : y -= last_move; break; // diagonal, we can come here from two sides, from left and right
			case 0x0111 : y -= 1; break;         // last_move indicates which of {b,c} we are 'attached' to
			case 0x1000 : x -= 1; break;
			case 0x1001 : x += last_move; break;
			case 0x1010 : y += 1; break;
			case 0x1011 : x += 1; break;
			case 0x1100 : x -= 1; break;
			case 0x1101 : x -= 1; break;
			case 0x1110 : y += 1; break;
			default:
				throw InternalError(_("in the ground/air"));
		}
		
		// add to shape and place a mark
		shape->points.push_back(new_intrusive2<ControlPoint>(
				double(x) / data.width,
				double(y) / data.height
			));
		if (x > old_x) data(old_x, y) |= MARKED; // mark when moving right -> only mark the top of the shape
		last_move = (x + y) - (old_x + old_y);
		old_x = x;
		old_y = y;
	} while (x != xs || y != ys); // we will end up in the start point
	
	// are we on the inside or the outside?
	if (data(x-2,y-1) & FULL) {
		shape->combine = SYMBOL_COMBINE_SUBTRACT;
	} else {
		shape->combine = SYMBOL_COMBINE_MERGE;
	}
	return shape;
}


SymbolP image_to_symbol(Image& img) {
	int w = img.GetWidth(), h = img.GetHeight();
	// 1. threshold the image
	greyscale(img);
	threshold(img.GetData(), w, h);
	// 2. read as many symbol shapes as we can
	ImageData data = {w,h,img.GetData()};
	SymbolP symbol(new Symbol);
	while (true) {
		SymbolShapeP shape = read_symbol_shape(data);
		if (!shape) break;
		symbol->parts.push_back(shape);
	}
	reverse(symbol->parts.begin(), symbol->parts.end());
	return symbol;
}

SymbolP import_symbol(Image& img) {
	SymbolP symbol;
	if (is_mse1_symbol(img)) {
		Image img2 = img.GetSubImage(wxRect(20,0,40,40));
		symbol = image_to_symbol(img2);
	} else if (img.GetWidth() > 100 || img.GetHeight() > 100) {
		// 100x100 ought to be enough, we trow out most afterwards data anyway
		Image resampled = img.Rescale(100,100);
		symbol = image_to_symbol(resampled);
	} else {
		symbol = image_to_symbol(img);
	}
	simplify_symbol(*symbol);
	return symbol;
}


// ----------------------------------------------------------------------------- : Simplify symbol

/// Finds corners, marks corners as LOCK_FREE, non-corners as LOCK_DIR
/** A corner is a point that has an angle between tangent greater then a treshold
 */
void mark_corners(SymbolShape& shape) {
	for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
		ControlPoint& current = *shape.getPoint(i);
		Vector2D before  = .6 * shape.getPoint(i-1)->pos + .2 * shape.getPoint(i-2)->pos + .1 * shape.getPoint(i-3)->pos + .1 * shape.getPoint(i-4)->pos;
		Vector2D after   = .6 * shape.getPoint(i+1)->pos + .2 * shape.getPoint(i+2)->pos + .1 * shape.getPoint(i+3)->pos + .1 * shape.getPoint(i+4)->pos;
		before = (before - current.pos).normalized();
		after  = (after  - current.pos).normalized();
		if (dot(before,after) >= -0.25f) {
			// corner
			current.lock = LOCK_FREE;
		} else {
			current.lock = LOCK_DIR;
		}
	}
}

/// Merge adjacent corners
/** Triangles will result in adjecent corners:
 *   XX
 *   XXXX   _ corner 1;
 *   XXXXXX _ corner 2;
 *   XXXX
 *   XX
 *
 *  Not all adjectent corners should be merged, for example
 *   X             _ 1
 *   XXXXXXXXXXXXX _ 2
 *  should be kept
 *
 *  The solution is to look at the tangent lines.
 *  Where these two lines (one for each corner) intersect,
 *   is the merged corner. If it is too far away, don't merge
 */
void merge_corners(SymbolShape& shape) {
	for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
		ControlPoint& cur  = *shape.getPoint(i);
		ControlPoint& prev = *shape.getPoint(i - 1);
		if (prev.lock != LOCK_FREE || cur.lock != LOCK_FREE) continue;
		// step 1. find tangent lines: try tangent lines to the first point, the second, etc.
		// and take the one that has the largest angle with ab, i.e. the smallest dot,
		// where ab is the line between the two corners
		Vector2D ab = cur.pos - prev.pos;
		double min_a_dot = 1e100, min_b_dot = 1e100;
		Vector2D a, b;
		for (int j = 0 ; j < 4 ; ++j) {
			Vector2D a_ = (shape.getPoint(i-j-1)->pos - prev.pos).normalized();
			Vector2D b_ = (shape.getPoint(i+j)->pos   - cur.pos).normalized();
			double a_dot =  dot(a_, ab);
			double b_dot = -dot(b_, ab);
			if (a_dot < min_a_dot) {
				min_a_dot = a_dot;
				a = a_;
			}
			if (b_dot < min_b_dot) {
				min_b_dot = b_dot;
				b = b_;
			}
		}
		// step 2. find intersection point, to solve:
		//  t a + ab = u b, solve for t,u
		// Gives us:
		//  t = ab cross b / b cross a
		double tden = max(0.00000001, cross(b,a));
		double t  = cross(ab,b) / tden;
		// do these tangent lines intersect, and not too far away?
		// if so, then the intersection point is the merged point
		if (t >= 0 && t < 20.0) {
			prev.pos += a * -t;
			shape.points.erase(shape.points.begin() + i);
			i -= 1;
		}
	}
}

/// Avarage/'blur' a symbol shape
void avarage(SymbolShape& shape) {
	// create a copy of the points
	vector<Vector2D> old_points;
	FOR_EACH(p, shape.points) {
		old_points.push_back(p->pos);
	}
	// avarage points
	for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
		ControlPoint& p = *shape.getPoint(i);
		if (p.lock == LOCK_DIR) {
			p.pos = .25 * old_points[mod(i-1, old_points.size())]
			      + .50 * p.pos
			      + .25 * old_points[mod(i+1, old_points.size())];
		}
	}
}

/// Convert a symbol shape to curves
void convert_to_curves(SymbolShape& shape) {
	// mark all segments as curves
	for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
		ControlPoint& cur  = *shape.getPoint(i);
		ControlPoint& next = *shape.getPoint(i + 1);
		cur.segment_after  = SEGMENT_CURVE;
		cur.segment_before = SEGMENT_CURVE;
		cur.delta_after   = (next.pos - cur.pos)  / 3.0;
		next.delta_before = (cur.pos  - next.pos) / 3.0;
	}
	// make the curves smooth by enforcing direction constraints
	FOR_EACH(p, shape.points) {
		p->onUpdateLock();
	}
}

/// Convert almost straight curves in a symbol shape to lines
void straighten(SymbolShape& shape) {
	const double treshold = 0.2;
	for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
		ControlPoint& cur  = *shape.getPoint(i);
		ControlPoint& next = *shape.getPoint(i + 1);
		Vector2D ab = (next.pos - cur.pos).normalized();
		Vector2D aa = cur.delta_after.normalized();
		Vector2D bb = next.delta_before.normalized();
		// if the area beneath the polygon formed by the handles is small
		// then it is a straight line
		double cpDot = abs(cross(aa,ab)) + abs(cross(bb,ab));
		if (cpDot < treshold) {
			cur.segment_after = next.segment_before = SEGMENT_LINE;
			cur.delta_after = next.delta_before = Vector2D();
			cur.lock = next.lock = LOCK_FREE;
		}
	}
}

/// Remove unneeded points between straight lines
void merge_lines(SymbolShape& shape) {
	for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
		ControlPoint& cur  = *shape.getPoint(i);
		if (cur.segment_before != cur.segment_after) continue;
		Vector2D a = shape.getPoint(i-1)->pos, b = cur.pos, c = shape.getPoint(i+1)->pos;
		Vector2D ab = (a-b).normalized();
		Vector2D bc = (b-c).normalized();
		double angle_len = abs(  atan2(ab.x,ab.y) - atan2(bc.x,bc.y))  * (a-c).lengthSqr();
		bool keep = angle_len >= .0001;
		if (!keep) {
			shape.points.erase(shape.points.begin() + i);
			i -= 1;
		}
	}
}

double cost_of_point_removal(SymbolShape& shape, int i);
void remove_point(SymbolShape& shape, int i);

/// Simplify a symbol shape by removing points
/** Always remove the point with the lowest cost,
 *  stop when the cost becomes too high
 */
void remove_points(SymbolShape& shape) {
	const double treshold = 0.0002; // maximum cost
	while (true) {
		// Find the point with the lowest cost of removal
		int best = -1;
		double best_cost = 1e100;
		for (int i = 0 ; (size_t)i < shape.points.size() ; ++i) {
			double cost = cost_of_point_removal(shape, i); 
			if (cost < best_cost) {
				best_cost = cost;
				best = i;
			}
		}
		if (best_cost > treshold) break;
		// ... and remove it
		remove_point(shape, best);
	}
}
/// Cost of removing point i from a symbol shape
double cost_of_point_removal(SymbolShape& shape, int i) {
	ControlPoint& cur  = *shape.getPoint(i);
	ControlPoint& prev = *shape.getPoint(i-1);
	ControlPoint& next = *shape.getPoint(i+1);
	if (cur.lock != LOCK_DIR) return 1e100; // don't remove corners
	
	Vector2D before = cur.delta_before;
	Vector2D after  = cur.delta_after;
	Vector2D ac     = prev.pos - next.pos;
	// Based on SinglePointRemoveAction
	double bl   = before.length() + 0.00001; // prevent division by 0
	double al   = after.length() + 0.00001;
	double totl = bl + al;
	// set new handle sizes
	Vector2D after0  = prev.delta_after * totl / bl;
	Vector2D before2 = next.delta_before * totl / al;
	// determine closest point on the merged curve
	BezierCurve c(prev.pos, prev.pos + after0, next.pos + before2, next.pos);
	double t = bl/totl;
	Vector2D np = cur.pos - c.pointAt(t);
	// cost is distance to new point * length of line ~= area added/removed from shape
	return np.length() * ac.length();
}
/// Remove a point from a bezier curve
/** See SinglePointRemoveAction for algorithm */
void remove_point(SymbolShape& shape, int i) {
	ControlPoint& cur  = *shape.getPoint(i);
	ControlPoint& prev = *shape.getPoint(i-1);
	ControlPoint& next = *shape.getPoint(i+1);
	Vector2D before = cur.delta_before;
	Vector2D after  = cur.delta_after;
	// Based on SinglePointRemoveAction
	double bl   = before.length() + 0.00001; // prevent division by 0
	double al   = after.length() + 0.00001;
	double totl = bl + al;
	// set new handle sizes
	prev.delta_after  *= totl / bl;
	next.delta_before *= totl / al;
	// remove
	shape.points.erase(shape.points.begin() + i);
}


void simplify_symbol_shape(SymbolShape& shape) {
	mark_corners(shape);
	merge_corners(shape);
	for (int i = 0 ; i < 3 ; ++i) {
		avarage(shape);
	}
	convert_to_curves(shape);
	remove_points(shape);
	straighten(shape);
	merge_lines(shape);
}

void simplify_symbol(Symbol& symbol) {
	FOR_EACH(pb, symbol.parts) {
		if (SymbolShape* p = pb->isSymbolShape()) {
			simplify_symbol_shape(*p);
		}
	}
}
