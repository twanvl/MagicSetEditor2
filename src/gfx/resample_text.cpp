//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gfx/gfx.hpp>
#include <util/error.hpp>
#include <gui/util.hpp> // clearDC_black

// ----------------------------------------------------------------------------- : Resampled text

// scaling factor to use when drawing resampled text
const int text_scaling = 4;

// Fills an image with the specified color
void fill_image(Image& image, const Color& color) {
	Byte* pos = image.GetData();
	Byte* end = image.GetData() + image.GetWidth() * image.GetHeight() * 3;
	while (pos != end) {
		pos[0] = color.Red();
		pos[1] = color.Green();
		pos[2] = color.Blue();
		pos += 3;
	}
}

// Downsamples the red channel of the input image to the alpha channel of the output image
// img_in must be text_scaling times as large as img_out
void downsample_to_alpha(Image& img_in, Image& img_out) {
	assert(img_in.GetWidth()  == img_out.GetWidth()  * text_scaling);
	assert(img_in.GetHeight() == img_out.GetHeight() * text_scaling);
	
	// scale in the x direction, this overwrites parts of the input image
	Byte* in  = img_in.GetData();
	Byte* out = img_in.GetData();
	int count = img_out.GetWidth() * img_in.GetHeight();
	for (int i = 0 ; i < count ; ++i) {
		int total = 0;
		for (int j = 0 ; j < text_scaling ; ++j) {
			total += in[3 * (j + text_scaling * i)];
		}
		out[i] = total / text_scaling;
	}
	
	// now scale in the y direction, and write to the output alpha
	img_out.InitAlpha();
	out   = img_out.GetAlpha();
	int line_size = img_out.GetWidth();
	for (int y = 0 ; y < img_out.GetHeight() ; ++y) {
		for (int x = 0 ; x < img_out.GetWidth() ; ++x) {
			int total = 0;
			for (int j = 0 ; j < text_scaling ; ++j) {
				total += in[x + line_size * (j + text_scaling * y)];
			}
			out[x + line_size * y] = total / text_scaling;
		}
	}
}

// Draw text by first drawing it using a larger font and then downsampling it
// optionally rotated by an angle
//  (w2,h2) = size of text
//  (wc,hc) = the corner where drawing should begin, (0,0) for top-left, (1,1) for bottom-right
void draw_resampled_text(DC& dc, const RealRect& rect, int wc, int hc, int angle, const String& text) {
	// enlarge slightly
	int w = rect.width + 1, h = rect.height + 1;
	// determine sub-pixel position
	int xi = rect.x, yi = rect.y;
	int xsub = text_scaling * (rect.x - xi), ysub = text_scaling * (rect.y - yi);
	// draw text
	Bitmap buffer(w * text_scaling, h * text_scaling, 24); // should be initialized to black
	wxMemoryDC mdc;
	mdc.SelectObject(buffer);
	clearDC_black(mdc);
	// now draw the text
	mdc.SetFont(dc.GetFont());
	mdc.SetTextForeground(*wxWHITE);
	mdc.DrawRotatedText(text, wc * w * text_scaling + xsub, hc * h * text_scaling + ysub, angle);
	// get image
	mdc.SelectObject(wxNullBitmap);
	Image img_large = buffer.ConvertToImage();
	// step 2. sample down
	Image img_small(w, h, false);
	fill_image(img_small, dc.GetTextForeground());
	downsample_to_alpha(img_large, img_small);
	// step 3. draw to dc
	dc.DrawBitmap(img_small, xi + wc * (rect.width - w), yi + hc * (rect.height - h));
}

