//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

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

void resample_and_clip(const Image& img_in, Image& img_out, wxRect rect) {
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
