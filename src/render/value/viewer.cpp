//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : ValueViewer

ValueViewer::ValueViewer(DataViewer& parent, const StyleP& style)
	: StyleListener(style), viewer(parent)
{}

Set& ValueViewer::getSet() const { return *viewer.getSet(); }

void ValueViewer::setValue(const ValueP& value) {
	assert(value->fieldP == styleP->fieldP); // matching field
	if (valueP == value) return;
	valueP = value;
	onValueChange();
}

bool ValueViewer::containsPoint(const RealPoint& p) const {
	return p.x >= 0
	    && p.y >= 0
	    && p.x <  styleP->width
	    && p.y <  styleP->height;
}
RealRect ValueViewer::boundingBox() const {
	return styleP->getExternalRect().grow(1);
}

Rotation ValueViewer::getRotation() const {
	return Rotation(getStyle()->angle, getStyle()->getExternalRect(), 1.0, getStretch());
}

void ValueViewer::drawFieldBorder(RotatedDC& dc) {
	if (viewer.drawBorders() && getField()->editable) {
		dc.SetPen(viewer.borderPen(isCurrent()));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(dc.getInternalRect().grow(dc.trInvS(1)));
	}
}

bool ValueViewer::nativeLook() const {
	return viewer.nativeLook();
}
bool ValueViewer::isCurrent() const {
	return viewer.focusedViewer() == this;
}

void ValueViewer::onStyleChange(int changes) {
	if (!(changes & CHANGE_ALREADY_PREPARED)) {
		viewer.redraw(*this);
	}
}
