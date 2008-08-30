//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <render/card/viewer.hpp>

DECLARE_TYPEOF_COLLECTION(wxPoint);

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
	return getMask().isOpaque(p, styleP->getSize());
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
		const AlphaMask& alpha_mask = getMask(dc);
		if (alpha_mask.isLoaded()) {
			// from mask
			vector<wxPoint> points;
			alpha_mask.convexHull(points);
			if (points.size() < 3) return;
			FOR_EACH(p, points) p = dc.trPixelNoZoom(RealPoint(p.x,p.y));
			dc.getDC().DrawPolygon((int)points.size(), &points[0]);
		} else {
			// simple rectangle
			dc.DrawRectangle(dc.getInternalRect().grow(dc.trInvS(1)));
		}
	}
}

const AlphaMask& ValueViewer::getMask(int w, int h) const {
	GeneratedImage::Options opts(w, h, &getStylePackage(), &getLocalPackage());
	return styleP->mask.get(opts);
}
const AlphaMask& ValueViewer::getMask(const Rotation& rot) const {
	return getMask((int)rot.trX(styleP->width), (int)rot.trY(styleP->height));
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
