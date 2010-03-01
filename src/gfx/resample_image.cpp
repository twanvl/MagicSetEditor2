//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Resample passes

// bitshift for fixed point numbers
//  higher is less error
//  we will get errors if 2^shift * imagesize becomes too large
const int shift = 32-10-8; // => max size = 1024, max alpha = 255

// Resample an image only in a single direction, either horizontally or vertically
/* Terms are based on x resampling (keeping the same number of lines):
 *  offset     = number of elements to skip at the start
 *  length     = length of a line
 *  delta      = number of elements between pixels in a lines
 *  lines      = number of lines
 *  line_delta = number of elements between the the first pixel of two lines
 *  1 element = 3 bytes in data, 1 byte in alpha
 */
void resample_pass(const Image& img_in, Image& img_out, int offset_in, int offset_out,
                   int length_in, int delta_in, int length_out, int delta_out,
                   int lines, int line_delta_in, int line_delta_out)
{
	bool alpha = img_in.HasAlpha();
	if (alpha && !img_out.HasAlpha()) img_out.InitAlpha();
	int out_fact = (length_out << shift) / length_in; // how much to output for 256 input = 1 pixel
	int out_rest = (length_out << shift) % length_in;
	// for each line
	for (int l = 0 ; l < lines ; ++l) {
		Byte* in  = img_in .GetData() + 3 * (offset_in  + l * line_delta_in);
		Byte* out = img_out.GetData() + 3 * (offset_out + l * line_delta_out);
		UInt in_rem = out_fact + out_rest; // remaining to input from the current input pixel
		
		if (alpha) {
			Byte* in_a  = img_in .GetAlpha() + (offset_in  + l * line_delta_in);
			Byte* out_a = img_out.GetAlpha() + (offset_out + l * line_delta_out);
			
			for (int x = 0 ; x < length_out ; ++x) {
				UInt out_rem = 1 << shift;
				UInt totR = 0, totG = 0, totB = 0, totA = 0;
				while (out_rem >= in_rem) {
					// eat a whole input pixel
					totR += in[0]   * in_rem * in_a[0]; // multiply by alpha
					totG += in[1]   * in_rem * in_a[0];
					totB += in[2]   * in_rem * in_a[0];
					totA += in_a[0] * in_rem;
					out_rem -= in_rem;
					in_rem = out_fact;
					in += 3*delta_in; in_a += delta_in;
				}
				if (out_rem > 0) {
					// eat a partial input pixel
					totR += in[0]   * out_rem * in_a[0];
					totG += in[1]   * out_rem * in_a[0];
					totB += in[2]   * out_rem * in_a[0];
					totA += in_a[0] * out_rem;
					in_rem -= out_rem;
				}
				// store
				if (totA) {
					out[0] = totR / totA;
					out[1] = totG / totA;
					out[2] = totB / totA;
					out_a[0] = totA >> shift;
				} else {
					out[0] = out[1] = out[2] = out_a[0] = 0; // div by 0 is bad
				}
				out += 3*delta_out; out_a += delta_out;
			}
			
		} else {
			// no alpha
			for (int x = 0 ; x < length_out ; ++x) {
				UInt out_rem = 1 << shift;
				UInt totR = 0, totG = 0, totB = 0;
				while (out_rem >= in_rem) {
					// eat a whole input pixel
					totR += in[0] * in_rem;
					totG += in[1] * in_rem;
					totB += in[2] * in_rem;
					out_rem -= in_rem;
					in_rem = out_fact;
					in += 3*delta_in;
				}
				if (out_rem > 0) {
					// eat a partial input pixel
					totR += in[0] * out_rem;
					totG += in[1] * out_rem;
					totB += in[2] * out_rem;
					in_rem -= out_rem;
				}
				// store
				out[0] = totR >> shift;
				out[1] = totG >> shift;
				out[2] = totB >> shift;
				out += 3*delta_out;
			}
		}
	}
}

// ----------------------------------------------------------------------------- : Resample

/* The algorithm first resizes in horizontally, then vertically,
 * the two passes are essentially the same:
 *  - for each row:
 *    - each input pixel becomes a fixed amount of output (in 1<<shift fixed point math)
 *    - for each output pixel:
 *      - _('eat') input pixels until the total is 1<<shift
 *      - write the total to the output pixel
 *  - to ensure the sum of all the pixel amounts is exacly width<<shift an extra rest amount
 *    is _('eaten') from the first pixel;
 *
 * Uses fixed point numbers
 */
void resample(const Image& img_in, Image& img_out) {
	resample_and_clip(img_in, img_out, wxRect(0, 0, img_in.GetWidth(), img_in.GetHeight()));
}
Image resample(const Image& img_in, int width, int height) {
	if (img_in.GetWidth() == width && img_in.GetHeight() == height) {
		return img_in; // already the right size
	} else {
		Image img_out(width,height,false);
		resample(img_in, img_out);
		return img_out;
	}
}

void resample_and_clip(const Image& img_in, Image& img_out, wxRect rect) {
	// mask to alpha
	if (img_in.HasMask() && !img_in.HasAlpha()) {
		const_cast<Image&>(img_in).InitAlpha();
	}
	// starting position in data
	int offset_in = (rect.x + img_in.GetWidth() * rect.y);
	if (img_out.GetHeight() == rect.height) {
		// no resizing vertically
		resample_pass(img_in,   img_out,  offset_in, 0, rect.width,  1,                   img_out .GetWidth(),  1,                   rect    .GetHeight(), img_in.GetWidth(), img_out .GetWidth());
	} else {
		Image img_temp(img_out.GetWidth(), rect.height, false);
		resample_pass(img_in,   img_temp, offset_in, 0, rect.width,  1,                   img_temp.GetWidth(),  1,                   rect    .GetHeight(), img_in.GetWidth(), img_temp.GetWidth());
		resample_pass(img_temp, img_out,  0,         0, rect.height, img_temp.GetWidth(), img_out .GetHeight(), img_temp.GetWidth(), img_temp.GetWidth(),  1,                 1);
	}
}


// ----------------------------------------------------------------------------- : Aspect ratio preserving

// fill an image with 100% transparent
void fill_transparent(Image& img) {
	if (!img.HasAlpha()) img.InitAlpha();
	memset(img.GetAlpha(), 0, img.GetWidth() * img.GetHeight());
}

void resample_preserve_aspect(const Image& img_in, Image& img_out) {
	int rheight = img_in.GetHeight() * img_out.GetWidth()  / img_in.GetWidth();
	int rwidth  = img_in.GetWidth()  * img_out.GetHeight() / img_in.GetHeight();
	// actual size of output
	if      (rheight < img_out.GetHeight()) rwidth  = img_out.GetWidth();
	else if (rwidth  < img_out.GetWidth())  rheight = img_out.GetHeight();
	else                                   {rwidth  = img_out.GetWidth(); rheight = img_out.GetHeight();}
	int dx = (img_out.GetWidth()  - rwidth)  / 2;
	int dy = (img_out.GetHeight() - rheight) / 2;
	// transparent background
	fill_transparent(img_out);
	// resample
	int offset_out = dx + img_out.GetWidth() * dy;
	Image img_temp(rwidth, img_in.GetHeight(), false);
	img_temp.InitAlpha();
	resample_pass(img_in,   img_temp, 0, 0,          img_in.GetWidth(),  1,                   rwidth,  1,                  img_in.GetHeight(), img_in.GetWidth(), img_temp.GetWidth());
	resample_pass(img_temp, img_out,  0, offset_out, img_in.GetHeight(), img_temp.GetWidth(), rheight, img_out.GetWidth(), rwidth,             1,                 1);
}

Image resample_preserve_aspect(const Image& img_in, int width, int height) {
	if (img_in.GetWidth() == width && img_in.GetHeight() == height) {
		return img_in; // already the right size
	} else {
		Image img_out(width,height,false);
		resample_preserve_aspect(img_in, img_out);
		return img_out;
	}
}

// ----------------------------------------------------------------------------- : Sharpening

void sharp_downsample(const Image& img_in, Image& img_out, int amount);

void sharp_resample(const Image& img_in, Image& img_out, int amount) {
	sharp_resample_and_clip(img_in, img_out, wxRect(0, 0, img_in.GetWidth(), img_in.GetHeight()), amount);
}

void sharp_resample_and_clip(const Image& img_in, Image& img_out, wxRect rect, int amount) {
	Image img_larger(img_out.GetWidth() * 2, img_out.GetHeight() * 2, false);
	resample_and_clip(img_in, img_larger, rect);
	sharp_downsample(img_larger, img_out, amount);
}

// Downsample an image to create a sharp result by applying a sharpening filter
// img_in must be twice as large as img_out
void sharp_downsample(const Image& img_in, Image& img_out, int amount) {
	assert(img_in.GetWidth()  == img_out.GetWidth() * 2);
	assert(img_in.GetHeight() == img_out.GetHeight() * 2);
	
	int width = img_out.GetWidth(), height = img_out.GetHeight();
	int line = width * 6;
	int center_weight = 201;
	int border_weight = amount;
	assert(4 * center_weight - 8 * border_weight > 0);
	
	Byte *in = img_in.GetData(), *out = img_out.GetData();
	Byte *al = nullptr, *outa = nullptr;
	if (img_in.HasAlpha()) {
		img_out.InitAlpha();
		al = img_in.GetAlpha();
		outa = img_out.GetAlpha();
	}
	
	for (int y = 0 ; y < height ; ++y) {
		for (int x = 0 ; x < width ; ++x) {
			// Filter using a kernel of the form
			/*     -1 -1
			 *  -1  c  c -1
			 *  -1  c  c -1
			 *     -1 -1
			 * But when we are near the edge ignore the pixel
			 */
			// when there is alpha, all weights are multiplied by 4*255
			int center_alpha = al ? al[0] + al[1] + al[width*2] + al[width*2+1] : 1;
			int weight = center_weight * center_alpha * 4;
			int sumR   = center_weight * center_alpha * (in[0] + in[3] + in[line+0] + in[line+3]);
			int sumG   = center_weight * center_alpha * (in[1] + in[4] + in[line+1] + in[line+4]);
			int sumB   = center_weight * center_alpha * (in[2] + in[5] + in[line+2] + in[line+5]);
			// edges
			if (x != 0) {
				int a = al ? border_weight * min(2 * (al[-1] + al[width*2-1]), center_alpha)
				           : border_weight;
				sumR -= a * (in[-3] + in[line-3]);
				sumG -= a * (in[-2] + in[line-2]);
				sumB -= a * (in[-1] + in[line-1]);
				weight -= a * 2;
			}
			if (x+1 != width) {
				int a = al ? border_weight * min(2 * (al[2] + al[width*2+2]), center_alpha)
				           : border_weight;
				sumR -= a * (in[6] + in[line+6]);
				sumG -= a * (in[7] + in[line+7]);
				sumB -= a * (in[8] + in[line+8]);
				weight -= a * 2;
			}
			if (y != 0) {
				int a = al ? border_weight * min(2 * (al[-width*2] + al[-width*2+1]), center_alpha)
				           : border_weight;
				sumR -= a * (in[-line+0] + in[-line+3]);
				sumG -= a * (in[-line+1] + in[-line+4]);
				sumB -= a * (in[-line+2] + in[-line+5]);
				weight -= a * 2;
			}
			if (y+1 != height) {
				int a = al ? border_weight * min(2 * (al[width*2*2] + al[width*2*2+1]), center_alpha)
				           : border_weight;
				sumR -= a * (in[line*2+0] + in[line*2+3]);
				sumG -= a * (in[line*2+1] + in[line*2+4]);
				sumB -= a * (in[line*2+2] + in[line*2+5]);
				weight -= a * 2;
			}
			// And then avarage the result into a single pixel (downsample by factor 2 in both dimensions)
			if (weight > 0) {
				out[0] = col( sumR / weight );
				out[1] = col( sumG / weight );
				out[2] = col( sumB / weight );
			} else {
				out[0] = out[1] = out[2] = 0;
			}
			if (al) {
				outa[0] = center_alpha / 4;
				outa += 1;
				al += 2;
			}
			// next pixel
			in  += 6;
			out += 3;
		}
		// skip a line
		in += line;
		if (al) al += width*2;
	}
}
