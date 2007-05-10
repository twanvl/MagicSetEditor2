//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/color.hpp>
#include <render/card/viewer.hpp>
#include <data/stylesheet.hpp>

DECLARE_TYPEOF_COLLECTION(ColorField::ChoiceP);

// ----------------------------------------------------------------------------- : ColorValueViewer

void ColorValueViewer::draw(RotatedDC& dc) {
	// draw in the value color
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(value().value());
	if (nativeLook()) {
		// native look
		// find name of color
		String color_name = _("Custom");
		if (field().default_script && value().value.isDefault()) {
			color_name = field().default_name;
		} else {
			FOR_EACH_CONST(c, field().choices) {
				if (value().value() == c->color) {
					color_name = capitalize(c->name);
					break;
				}
			}
		}
		// draw name and color
		dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.DrawRectangle(RealRect(style().left, style().top, 40, style().height));
		dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(style().getRect().move(40, 0, -40, 0));
		dc.DrawText(color_name, style().getPos() + RealSize(43, 3));
	} else {
		// is there a mask?
		loadMask(dc);
		if (alpha_mask) {
			Image img(alpha_mask->size.x, alpha_mask->size.y);
			fill_image(img, value().value());
			alpha_mask->setAlpha(img);
			dc.DrawImage(img, style().getPos());
		} else {
			// do we need clipping?
			bool clip = style().left_width < style().width  && style().right_width  < style().width &&
						style().top_width  < style().height && style().bottom_width < style().height;
			if (clip) {
				// clip away the inside of the rectangle
				wxRegion r = dc.tr(style().getRect()).toRect();
				r.Subtract(dc.tr(RealRect(
					style().left + style().left_width,
					style().top  + style().top_width,
					style().width  - style().left_width - style().right_width,
					style().height - style().top_width  - style().bottom_width
				)));
				dc.getDC().SetClippingRegion(r);
			}
			dc.DrawRoundedRectangle(style().getRect(), style().radius);
			if (clip) dc.getDC().DestroyClippingRegion();
		}
	}
}

bool ColorValueViewer::containsPoint(const RealPoint& p) const {
	// distance to each side
	double left = p.x - style().left,  right  = style().left + style().width  - p.x - 1;
	double top  = p.y - style().top,   bottom = style().top  + style().height - p.y - 1;
	if (left < 0 || right < 0 || top < 0 || bottom < 0 ||                 // outside bounding box
	    (left >= style().left_width && right  >= style().right_width &&   // outside horizontal border
	     top  >= style().top_width  && bottom >= style().bottom_width)) { // outside vertical border
		return false;
	}
	// check against mask
	if (!style().mask_filename().empty()) {
		loadMask(viewer.getRotation());
		return !alpha_mask || !alpha_mask->isTransparent((int)left, (int)top);
	} else {
		return true;
	}
}

void ColorValueViewer::onStyleChange() {
	alpha_mask = AlphaMaskP();
	viewer.redraw(*this);
}

void ColorValueViewer::loadMask(const Rotation& rot) const {
	if (style().mask_filename().empty()) return; // no mask
	int w = (int) rot.trS(style().width), h = (int) rot.trS(style().height);
	if (alpha_mask && alpha_mask->size == wxSize(w,h)) return; // mask loaded and right size
	// (re) load the mask
	Image image;
	InputStreamP image_file = viewer.stylesheet->openIn(style().mask_filename);
	if (image.LoadFile(*image_file)) {
		Image resampled(w,h);
		resample(image, resampled);
		alpha_mask = new_shared1<AlphaMask>(resampled);
	}
}
