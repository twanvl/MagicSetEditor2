//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/card/viewer.hpp>
#include <data/field.hpp>

// ----------------------------------------------------------------------------- : DataViewer


// ----------------------------------------------------------------------------- : Drawing

void DataViewer::draw(DC& dc) {
}
void DataViewer::draw(RotatedDC& dc) {
}

// ----------------------------------------------------------------------------- : Utility for ValueViewers

bool   DataViewer::nativeLook()    const { return false; }
bool   DataViewer::drawBorders()   const { return false; }
wxPen  DataViewer::borderPen(bool) const { return wxPen(); }
Value* DataViewer::focusedValue()  const { return nullptr; }

// ----------------------------------------------------------------------------- : Setting data


// ----------------------------------------------------------------------------- : Viewers

ValueViewerP DataViewer::makeViewer(const StyleP& style) {
	return style->makeViewer(*this, style);
}

void DataViewer::onAction(const Action&, bool undone) {
	// TODO
}
