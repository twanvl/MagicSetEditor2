//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/color.hpp>
#include <render/card/viewer.hpp>

DECLARE_TYPEOF_COLLECTION(ColorField::ChoiceP);

// ----------------------------------------------------------------------------- : ColorValueViewer

IMPLEMENT_VALUE_VIEWER(Color);

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
		dc.DrawRectangle(RealRect(0, 0, 40, dc.getHeight()));
		dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(RealRect(40, 0, dc.getWidth()-40, dc.getHeight()));
		dc.DrawText(color_name, RealPoint(43, 3));
	} else {
		// is there a mask?
		const AlphaMask& alpha_mask = getMask(dc);
		if (alpha_mask.isLoaded()) {
			dc.DrawImage(alpha_mask.colorImage(value().value()), RealPoint(0,0), style().combine);
		} else {
			// do we need clipping?
			bool clip = style().left_width < style().width  && style().right_width  < style().width &&
						style().top_width  < style().height && style().bottom_width < style().height;
			if (clip) {
				// clip away the inside of the rectangle
				wxRegion r = dc.trRectToRegion(style().getInternalRect());
				r.Subtract(dc.trRectToRegion(RealRect(
					style().left_width,
					style().top_width,
					style().width  - style().left_width - style().right_width,
					style().height - style().top_width  - style().bottom_width
				)));
				dc.getDC().SetClippingRegion(r);
			}
			dc.DrawRoundedRectangle(style().getInternalRect(), style().radius);
			if (clip) dc.getDC().DestroyClippingRegion();
		}
		drawFieldBorder(dc);
	}
}

bool ColorValueViewer::containsPoint(const RealPoint& p) const {
	// check against mask
	const AlphaMask& alpha_mask = getMask();
	if (alpha_mask.isLoaded()) {
		// check against mask
		return alpha_mask.isOpaque(p, style().getSize());
	} else {
		double left = p.x, right  = style().width  - p.x - 1;
		double top  = p.y, bottom = style().height - p.y - 1;
		if (left < 0 || right < 0 || top < 0 || bottom < 0) return false;  // outside bounding box
		// check against border
		return left < style().left_width || right  < style().right_width   // inside horizontal border
			|| top  < style().top_width  || bottom < style().bottom_width; // inside vertical border
	}
}
