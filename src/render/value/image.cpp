//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/image.hpp>
#include <render/card/viewer.hpp>
#include <data/set.hpp>
#include <gui/util.hpp>

// ----------------------------------------------------------------------------- : ImageValueViewer

void ImageValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	// try to load image
	if (!bitmap.Ok() && !value().filename.empty()) {
		try {
			InputStreamP image_file = getSet().openIn(value().filename);
			Image image;
			if (image.LoadFile(*image_file)) {
				image.Rescale(dc.trS(style().width), dc.trS(style().height));
				// apply mask to image
/*				loadMask(dc);
				if (alpha_mask) alpha_mask->setAlpha(image);
*/				bitmap = Bitmap(image);
			}
		} catch (Error e) {
			handle_error(e, false, false); // don't handle now, we are in onPaint
		}
	}
	// if there is no image, generate a placeholder, only if there is enough room for it
	if (!bitmap.Ok() && style().width > 40) {
		bitmap = imagePlaceholder(dc, dc.trS(style().width), dc.trS(style().height), viewer.drawEditing());
/*		loadMask(dc);
		if (alpha_mask) alpha_mask->setAlpha(bitmap);
/*		if (alphaMask) {
			// convert to image and apply alpha
			Image image = bmp.ConvertToImage();
			alpha_mask->setAlpha(image);
			bitmap = image;
		}
*/	}
	// draw image, if any
	if (bitmap.Ok()) {
		dc.DrawBitmap(bitmap, style().getPos());
	}
}

bool ImageValueViewer::containsPoint(const RealPoint& p) const {
	int x = p.x - style().left;
	int y = p.y - style().top;
	if (x < 0 || y < 0 || x >= (int)style().width || y >= (int)style().height) {
		return false; // outside rectangle
	}
/*	// check against mask
	if (!style->maskFilename.value.empty()) {
		RotatedObject rot(viewer.getRotation());
		loadMask(rot);
		return !alphaMask->isTransparent(x, y);
	} else {
		return true;
	}*/
	return true;
}

void ImageValueViewer::onValueChange() {
	bitmap = Bitmap();
}

void ImageValueViewer::onStyleChange() {
	bitmap = Bitmap();
//	alpha_mask = AlphaMaskP();
}

Bitmap ImageValueViewer::imagePlaceholder(const Rotation& rot, UInt w, UInt h, bool editing) {
	// Bitmap and memory dc
	Bitmap bmp(w, h, 24);
	wxMemoryDC mdc;
	mdc.SelectObject(bmp);
	RealRect rect(0,0,w,h);
	RotatedDC dc(mdc, 0, rect, 1.0, true);
	// Draw checker background
	draw_checker(dc, rect);
	// Draw text
	if (editing) {
		// only when in editor mode
		for (UInt size = 12 ; size > 2 ; --size) {
			dc.SetFont(wxFont(size, wxSWISS, wxNORMAL, wxNORMAL));
			RealSize rs = dc.GetTextExtent(_("double click to load image"));
			if (rs.width <= w - 10 && rs.height < h - 10) {
				// text fits
				dc.SetTextForeground(*wxBLACK);
				dc.DrawText(_("double click to load image"), align_in_rect(ALIGN_MIDDLE_CENTER, rs, rect));
				break;
			}
		}
	}
	// Done
	mdc.SelectObject(wxNullBitmap);
	return bmp;
}
