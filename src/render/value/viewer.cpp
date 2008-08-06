//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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

Package& ValueViewer::getStylePackage() const { return viewer.getStylePackage(); }
Package& ValueViewer::getLocalPackage() const { return viewer.getLocalPackage(); }

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

bool ValueViewer::setFieldBorderPen(RotatedDC& dc) {
	if (!getField()->editable) return false;
	DrawWhat what = viewer.drawWhat(this);
	if (!(what & DRAW_BORDERS)) return false;
	dc.SetPen( (what & DRAW_ACTIVE)
	               ? wxPen(Color(0,128,255),   1, wxSOLID)
	               : wxPen(Color(128,128,128), 1, wxDOT)
	         );
	return true;
}

void ValueViewer::drawFieldBorder(RotatedDC& dc) {
	if (setFieldBorderPen(dc)) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(dc.getInternalRect().grow(dc.trInvS(1)));
	}
}

void ValueViewer::redraw() {
	viewer.redraw(*this);
}

bool ValueViewer::nativeLook() const {
	return viewer.nativeLook();
}
bool ValueViewer::isCurrent() const {
	return viewer.viewerIsCurrent(this);
}

void ValueViewer::onStyleChange(int changes) {
	if (!(changes & CHANGE_ALREADY_PREPARED)) {
		viewer.redraw(*this);
	}
}
