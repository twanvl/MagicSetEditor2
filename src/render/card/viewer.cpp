//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/card/viewer.hpp>
#include <render/value/viewer.hpp>
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <data/card.hpp>
#include <data/field.hpp>

DECLARE_TYPEOF_COLLECTION(ValueViewerP);

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

void DataViewer::setCard(Card& card) {
	assert(set);
	setStyles(set->stylesheet->card_style);
	setData(card.data);
}

// ----------------------------------------------------------------------------- : Viewers

void DataViewer::setStyles(IndexMap<FieldP,StyleP>& styles) {
}

void DataViewer::setData(IndexMap<FieldP,ValueP>& values) {
	FOR_EACH(v, viewers) {
		v->setValue(values[v->getField()]);
	}
}


ValueViewerP DataViewer::makeViewer(const StyleP& style) {
	return style->makeViewer(*this, style);
}

void DataViewer::onAction(const Action&, bool undone) {
	// TODO
}
