//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/image.hpp>
#include <render/card/viewer.hpp>
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <gui/util.hpp>

// ----------------------------------------------------------------------------- : ImageValueViewer

void ImageValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	// reset?
	int w = (int)dc.trX(style().width), h = (int)dc.trY(style().height);
	int a = dc.trAngle(style().angle);
	if (bitmap.Ok() && (a != angle || bitmap.GetWidth() != w || bitmap.GetHeight() != h)) {
		bitmap = Bitmap();
	}
	// try to load image
	if (!bitmap.Ok()) {
		angle = a;
		is_default = false;
		Image image;
		loadMask(dc);
		// load from file
		if (!value().filename.empty()) {
			try {
				InputStreamP image_file = getSet().openIn(value().filename);
				if (image.LoadFile(*image_file)) {
					image.Rescale(w, h);
				}
			} catch (Error e) {
				handle_error(e, false, false); // don't handle now, we are in onPaint
			}
		}
		// nice placeholder
		if (!image.Ok() && style().default_image.isReady()) {
			image = style().default_image.generate(GeneratedImage::Options(w, h, viewer.stylesheet.get(), &getSet()));
			is_default = true;
			if (viewer.drawEditing()) {
				bitmap = imagePlaceholder(dc, w, h, image, viewer.drawEditing());
				if (alpha_mask || a) {
					image = bitmap.ConvertToImage(); // we need to convert back to an image
				} else {
					image = Image();
				}
			}
		}
		// checkerboard placeholder
		if (!image.Ok() && !bitmap.Ok() && style().width > 40) {
			// placeholder bitmap
			bitmap = imagePlaceholder(dc, w, h, wxNullImage, viewer.drawEditing());
			if (alpha_mask || a) {
				// we need to convert back to an image
				image = bitmap.ConvertToImage();
			}
		}
		// done
		if (image.Ok()) {
			// apply mask and rotate
			if (alpha_mask) alpha_mask->setAlpha(image);
			image = rotate_image(image, angle);
			bitmap = Bitmap(image);
		}
	}
	// draw image, if any
	if (bitmap.Ok()) {
		dc.DrawPreRotatedBitmap(bitmap, style().getPos());
	}
	/*
	// if there is no image, generate a placeholder
	if (!bitmap.Ok()) {
		UInt w = (UInt)dc.trX(style().width), h = (UInt)dc.trY(style().height);
		loadMask(dc);
		if (style().default_image.isReady()) {
			// we have a script to use for the default image
			Image img = style().default_image.generate(GeneratedImage::Options(w, h, viewer.stylesheet.get(), &getSet()));
			if (viewer.drawEditing()) {
				bitmap = imagePlaceholder(dc, w, h, img, viewer.drawEditing());
				if (alpha_mask) alpha_mask->setAlpha(bitmap);
			} else {
				if (alpha_mask) alpha_mask->setAlpha(img);
				bitmap = Bitmap(img);
			}
		} else if (style().width > 40) {
			// still not okay, use a checkered image, but only if there is enough room for it
			bitmap = imagePlaceholder(dc, w, h, wxNullImage, viewer.drawEditing());
			if (alpha_mask) alpha_mask->setAlpha(bitmap);
		}
	}
	// draw image, if any
	if (bitmap.Ok()) {
		dc.DrawBitmap(bitmap, style().getPos());
	}
	*/
}

bool ImageValueViewer::containsPoint(const RealPoint& p) const {
	double x = p.x - style().left;
	double y = p.y - style().top;
	if (x < 0 || y < 0 || x >= style().width || y >= style().height) {
		return false; // outside rectangle
	}
	// check against mask
	if (!style().mask_filename().empty()) {
		loadMask(viewer.getRotation());
		Rotation rot = viewer.getRotation();
		return !alpha_mask || !alpha_mask->isTransparent((int)rot.trX(x), (int)rot.trY(y));
	} else {
		return true;
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
	if (changes & CHANGE_MASK) alpha_mask = AlphaMaskP();
	ValueViewer::onStyleChange(changes);
}

void ImageValueViewer::loadMask(const Rotation& rot) const {
	if (style().mask_filename().empty()) return; // no mask
	int w = (int) rot.trX(style().width), h = (int) rot.trY(style().height);
	if (alpha_mask && alpha_mask->size == wxSize(w,h)) return; // mask loaded and right size
	// (re) load the mask
	Image image;
	InputStreamP image_file = viewer.stylesheet->openIn(style().mask_filename);
	if (image.LoadFile(*image_file)) {
		Image resampled(w,h);
		resample(image, resampled);
		alpha_mask = new_intrusive1<AlphaMask>(resampled);
	}
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
