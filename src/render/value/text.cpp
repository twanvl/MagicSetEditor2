//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/text.hpp>
#include <render/card/viewer.hpp>
#include <data/stylesheet.hpp>

// ----------------------------------------------------------------------------- : TextValueViewer

IMPLEMENT_VALUE_VIEWER(Text);

bool TextValueViewer::prepare(RotatedDC& dc) {
	if (!style().mask_filename.empty() && !style().mask.ok()) {
		// load contour mask
		Image image;
		InputStreamP image_file = viewer.stylesheet->openIn(style().mask_filename);
		if (image.LoadFile(*image_file)) {
			style().mask.load(image);
		}
	}
	return v.prepare(dc, value().value(), style(), viewer.getContext());
}

void TextValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	if (!v.prepared()) {
		v.prepare(dc, value().value(), style(), viewer.getContext());
		dc.setStretch(getStretch());
	}
	if (viewer.drawFocus() && isCurrent()) {
		v.draw(dc, style(), DRAW_ACTIVE);
	}
	v.draw(dc, style(), (DrawWhat)(
		  DRAW_NORMAL
		| (viewer.drawBorders()              ? DRAW_BORDERS : 0)
		));
}

void TextValueViewer::onValueChange() {
	v.reset(false);
}

void TextValueViewer::onStyleChange(int changes) {
	v.reset(true);
	ValueViewer::onStyleChange(changes);
}

void TextValueViewer::onAction(const Action&, bool undone) {
	v.reset(true);
}

double TextValueViewer::getStretch() const {
	return v.prepared() ? style().getStretch() : 1.0;
}
