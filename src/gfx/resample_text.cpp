//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gfx/gfx.hpp>
#include <util/error.hpp>
#include <gui/util.hpp> // clearDC_black

void blur_image(const Image& img_in, Image& img_out);

// ----------------------------------------------------------------------------- : Resampled text

// scaling factor to use when drawing resampled text
const int text_scaling = 4;

// Downsamples the red channel of the input image to the alpha channel of the output image
// img_in must be text_scaling times as large as img_out
void downsample_to_alpha(Image& img_in, Image& img_out) {
	Byte* temp = nullptr;
	Byte* in  = img_in.GetData();
	Byte* out = img_in.GetData();
	// scale in the x direction, this overwrites parts of the input image
	if (img_in.GetWidth() == img_out.GetWidth() * text_scaling) {
		// no stretching
		int count = img_out.GetWidth() * img_in.GetHeight();
		for (int i = 0 ; i < count ; ++i) {
			int total = 0;
			for (int j = 0 ; j < text_scaling ; ++j) {
				total += in[3 * (j + text_scaling * i)];
			}
			out[i] = total / text_scaling;
		}
	} else {
		// resample to buffer
		temp = new Byte[img_out.GetWidth() * img_in.GetHeight()];
		out = temp;
		// custom stretch, see resample_image.cpp
		const int shift = 32-12-8; // => max size = 4096, max alpha = 255
		int w1 = img_in.GetWidth(), w2 = img_out.GetWidth(), h = img_in.GetHeight();
		int out_fact = (w2 << shift) / w1; // how much to output for 256 input = 1 pixel
		int out_rest = (w2 << shift) % w1;
		for (int y = 0 ; y < h ; ++y) {
			int in_rem = out_fact + out_rest;
			for (int x = 0 ; x < w2 ; ++x) {
				int out_rem = 1 << shift;
				int tot = 0;
				while (out_rem >= in_rem) {
					// eat a whole input pixel
					tot += *in * in_rem;
					out_rem -= in_rem;
					in_rem = out_fact;
					in += 3;
				}
				if (out_rem > 0) {
					// eat a partial input pixel
					tot += *in * out_rem;
					in_rem -= out_rem;
				}
				// store
				*out = tot >> shift;
				out += 1;
			}
		}
		in = temp;
	}
	
	// now scale in the y direction, and write to the output alpha
	img_out.InitAlpha();
	out = img_out.GetAlpha();
	int line_size = img_out.GetWidth();
	int h = img_out.GetHeight();
	if (img_in.GetHeight() == h * text_scaling) {
		// no stretching
		for (int y = 0 ; y < h ; ++y) {
			for (int x = 0 ; x < line_size ; ++x) {
				int total = 0;
				for (int j = 0 ; j < text_scaling ; ++j) {
					total += in[x + line_size * (j + text_scaling * y)];
				}
				out[x + line_size * y] = total / text_scaling;
			}
		}
	} else {
		const int shift = 32-12-8; // => max size = 4096, max alpha = 255
		int h1 = img_in.GetHeight(), w = img_out.GetWidth();
		int out_fact = (h << shift) / h1; // how much to output for 256 input = 1 pixel
		int out_rest = (h << shift) % h1;
		for (int x = 0 ; x < w ; ++x) {
			int in_rem = out_fact + out_rest;
			for (int y = 0 ; y < h ; ++y) {
				int out_rem = 1 << shift;
				int tot = 0;
				while (out_rem >= in_rem) {
					// eat a whole input pixel
					tot += *in * in_rem;
					out_rem -= in_rem;
					in_rem = out_fact;
					in += line_size;
				}
				if (out_rem > 0) {
					// eat a partial input pixel
					tot += *in * out_rem;
					in_rem -= out_rem;
				}
				// store
				*out = tot >> shift;
				out += line_size;
			}
			in  = in  - h1 * line_size + 1;
			out = out - h  * line_size + 1;
		}
	}
	
	delete[] temp;
}

// simple blur
int blur_alpha_pixel(Byte* in, int x, int y, int width, int height) {
	return (2 * (                      in[0])      + // center
	        (x == 0          ? in[0] : in[-1])     + // left
	        (y == 0          ? in[0] : in[-width]) + // up
	        (x == width - 1  ? in[0] : in[1])      + // right
	        (y == height - 1 ? in[0] : in[width])    // down
	       ) / 6;
}

// TODO: move me?
void blur_image_alpha(Image& img) {
	int width = img.GetWidth(), height = img.GetHeight();
	Byte* data = img.GetAlpha();
	for (int y = 0 ; y < height ; ++y) {
		for (int x = 0 ; x < width ; ++x) {
			*data = blur_alpha_pixel(data, x, y, width, height);
			++data;
		}
	}
}

// Draw text by first drawing it using a larger font and then downsampling it
// optionally rotated by an angle
//  (w2,h2) = size of text
//  (wc,hc) = the corner where drawing should begin, (0,0) for top-left, (1,1) for bottom-right
void draw_resampled_text(DC& dc, const RealRect& rect, double stretch, int wc, int hc, int angle, const String& text, int blur_radius, int repeat) {
	// enlarge slightly; some fonts are larger then the GetTextExtent tells us (especially italic fonts)
	int w = static_cast<int>(rect.width) + 3 + 2 * blur_radius, h = static_cast<int>(rect.height) + 1 + 2 * blur_radius;
	// determine sub-pixel position
	int xi = static_cast<int>(rect.x), yi = static_cast<int>(rect.y);
	int xsub = static_cast<int>(text_scaling * (rect.x - xi)), ysub = static_cast<int>(text_scaling * (rect.y - yi));
	// draw text
	Bitmap buffer(w * text_scaling, h * text_scaling, 24); // should be initialized to black
	wxMemoryDC mdc;
	mdc.SelectObject(buffer);
	clearDC_black(mdc);
	// now draw the text
	mdc.SetFont(dc.GetFont());
	mdc.SetTextForeground(*wxWHITE);
	mdc.DrawRotatedText(text, (wc * w + blur_radius) * text_scaling + xsub, (hc * h + blur_radius) * text_scaling + ysub, angle);
	// get image
	mdc.SelectObject(wxNullBitmap);
	Image img_large = buffer.ConvertToImage();
	// step 2. sample down
	if (!sideways(angle)) w *= stretch;
	else                  h *= stretch;
	Image img_small(w, h, false);
	fill_image(img_small, dc.GetTextForeground());
	downsample_to_alpha(img_large, img_small);
	// blur
	for (int i = 0 ; i < blur_radius ; ++i) {
		blur_image_alpha(img_small);
	}
	// step 3. draw to dc
	for (int i = 0 ; i < repeat ; ++i) {
		dc.DrawBitmap(img_small, xi + static_cast<int>(wc * (rect.width  - w)) - blur_radius,
		                         yi + static_cast<int>(hc * (rect.height - h)) - blur_radius);
	}
}

