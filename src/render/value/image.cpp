//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/image.hpp>
#include <render/card/viewer.hpp>
#include <gui/util.hpp>

// ----------------------------------------------------------------------------- : ImageValueViewer

IMPLEMENT_VALUE_VIEWER(Image);

void ImageValueViewer::draw(RotatedDC& dc) {
	DrawWhat what = viewer.drawWhat(this);
	// reset?
	int w = max(0,(int)dc.trX(style().width)), h = max(0,(int)dc.trY(style().height));
	int a = dc.trAngle(0); //% TODO : Add getAngle()?
	const AlphaMask& alpha_mask = getMask(w,h);
	if (bitmap.Ok() && (a != angle || size.width != w || size.height != h)) {
		bitmap = Bitmap();
	}
	// try to load image
	if (!bitmap.Ok()) {
		angle = a;
		is_default = false;
		Image image;
		// load from file
		if (!value().filename.empty()) {
			try {
				InputStreamP image_file = getLocalPackage().openIn(value().filename);
				if (image.LoadFile(*image_file)) {
					image.Rescale(w, h);
				}
			} catch (Error e) {
				handle_error(e, false, false); // don't handle now, we are in onPaint
			}
		}
		// nice placeholder
		if (!image.Ok() && style().default_image.isReady()) {
			image = style().default_image.generate(GeneratedImage::Options(w, h, &getStylePackage(), &getLocalPackage()));
			is_default = true;
			if (what & DRAW_EDITING) {
				bitmap = imagePlaceholder(dc, w, h, image, what & DRAW_EDITING);
				if (alpha_mask.isLoaded() || a) {
					image = bitmap.ConvertToImage(); // we need to convert back to an image
				} else {
					image = Image();
				}
			}
		}
		// checkerboard placeholder
		if (!image.Ok() && !bitmap.Ok() && style().width > 40) {
			// placeholder bitmap
			bitmap = imagePlaceholder(dc, w, h, wxNullImage, what & DRAW_EDITING);
			if (alpha_mask.isLoaded() || a) {
				// we need to convert back to an image
				image = bitmap.ConvertToImage();
			}
		}
		// done
		if (image.Ok()) {
			// apply mask and rotate
			alpha_mask.setAlpha(image);
			size = RealSize(image);
			image = rotate_image(image, angle);
			bitmap = Bitmap(image);
		}
	}
	// border
	drawFieldBorder(dc);
	// draw image, if any
	if (bitmap.Ok()) {
		dc.DrawPreRotatedBitmap(bitmap, dc.getInternalRect());
	}
}

void ImageValueViewer::onValueChange() {
	bitmap = Bitmap();
}

void ImageValueViewer::onStyleChange(int changes) {
	if ((changes & CHANGE_MASK) ||
	    ((changes & CHANGE_DEFAULT) && is_default)) {
		bitmap = Bitmap();
	}
	ValueViewer::onStyleChange(changes);
}

// is an image very light?
bool very_light(const Image& image) {
	int w = image.GetWidth(), h = image.GetHeight();
	if (w*h<1) return false;
	Byte* data = image.GetData();
	int middle = w / 2 + (h*w) / 2;
	int total = (int)data[3 * middle] + (int)data[3 * middle + 1] + (int)data[3 * middle + 2];
	return total >= 210 * 3;
}

Bitmap ImageValueViewer::imagePlaceholder(const Rotation& rot, UInt w, UInt h, const Image& background, bool editing) {
	// Bitmap and memory dc
	Bitmap bmp(w, h, 24);
	wxMemoryDC mdc;
	mdc.SelectObject(bmp);
	RealRect rect(0,0,w,h);
	RotatedDC dc(mdc, 0, rect, 1.0, QUALITY_AA);
	// Draw (checker) background
	if (background.Ok()) {
		dc.DrawImage(background, RealPoint(0,0));
	} else {
		draw_checker(dc, rect);
	}
	// Draw text
	if (editing) {
		// only when in editor mode
		for (UInt size = 12 ; size > 2 ; --size) {
			dc.SetFont(wxFont(size, wxSWISS, wxNORMAL, wxNORMAL));
			RealSize rs = dc.GetTextExtent(_("double click to load image"));
			if (rs.width <= w - 10 && rs.height < h - 10) {
				// text fits
				RealPoint pos = align_in_rect(ALIGN_MIDDLE_CENTER, rs, rect);
				bool black_on_white = !background.Ok() || very_light(background);
				dc.SetTextForeground(black_on_white ? *wxWHITE : *wxBLACK);
				dc.DrawText(_("double click to load image"), pos, 2, 4); // blurred
				dc.SetTextForeground(black_on_white ? *wxBLACK : *wxWHITE);
				dc.DrawText(_("double click to load image"), pos);
				break;
			}
		}
	}
	// Done
	mdc.SelectObject(wxNullBitmap);
	return bmp;
}
