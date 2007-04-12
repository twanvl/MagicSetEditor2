//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/text.hpp>
#include <render/card/viewer.hpp>
#include <data/stylesheet.hpp>

// ----------------------------------------------------------------------------- : TextValueViewer

void TextValueViewer::draw(RotatedDC& dc) {
	if (!style().mask_filename.empty() && !style().mask.ok()) {
		// load contour mask
		Image image;
		InputStreamP image_file = viewer.stylesheet->openIn(style().mask_filename);
		if (image.LoadFile(*image_file)) {
			style().mask.load(image);
		}
	}
	drawFieldBorder(dc);
	v.prepare(dc, value().value(), style(), viewer.getContext());
	v.draw(dc, style(), (DrawWhat)(
		  DRAW_NORMAL
		| (viewer.drawBorders()              ? DRAW_BORDERS : 0)
		| (viewer.drawFocus() && isCurrent() ? DRAW_ACTIVE  : 0)
		));
}

void TextValueViewer::onValueChange() {
	v.reset();
}

void TextValueViewer::onStyleChange() {
	v.reset();
}
