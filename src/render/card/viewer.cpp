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
#include <data/settings.hpp>

DECLARE_TYPEOF_COLLECTION(ValueViewerP);
typedef IndexMap<FieldP,StyleP> IndexMap_FieldP_StyleP;
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_StyleP);

// ----------------------------------------------------------------------------- : DataViewer


// ----------------------------------------------------------------------------- : Drawing

void DataViewer::draw(DC& dc) {
	StyleSheetP stylesheet = set->stylesheetFor(card);
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	RotatedDC rdc(dc, ss.card_angle(), stylesheet->getCardRect(), ss.card_zoom(), ss.card_anti_alias() && !nativeLook());
	draw(rdc, set->stylesheet->card_background);
}
void DataViewer::draw(RotatedDC& dc, const Color& background) {
	if (!set)  return; // no set specified, don't draw anything
	// fill with background color
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(background);
	dc.DrawRectangle(dc.getInternalRect());
	// update style scripts
	if (card) set->updateFor(card);
	// draw values
	FOR_EACH(v, viewers) { // draw low z index fields first
		if (v->getStyle()->visible) {// visible
			drawViewer(dc, *v);
		}
	}
}
void DataViewer::drawViewer(RotatedDC& dc, ValueViewer& v) {
	v.draw(dc);
}

// ----------------------------------------------------------------------------- : Utility for ValueViewers

bool   DataViewer::nativeLook()    const { return false; }
bool   DataViewer::drawBorders()   const { return false; }
bool   DataViewer::drawEditing()   const { return false; }
wxPen  DataViewer::borderPen(bool) const { return wxPen(); }
ValueViewer* DataViewer::focusedViewer() const { return nullptr; }
Context& DataViewer::getContext()  const { return set->getContext(); }

// ----------------------------------------------------------------------------- : Setting data

void DataViewer::setCard(const CardP& card) {
	assert(set);
	this->card = card;
	setStyles(set->stylesheet->card_style);
	setData(card->data);
}

// ----------------------------------------------------------------------------- : Viewers

struct CompareViewer {
	bool operator() (const ValueViewerP& a, const ValueViewerP& b) {
		return a->getStyle()->z_index < b->getStyle()->z_index;
	}
};

void DataViewer::setStyles(IndexMap<FieldP,StyleP>& styles) {
	if (!viewers.empty() && styles.contains(viewers.front()->getStyle())) {
		// already using these styles
		return;
	}
	// create viewers
	viewers.clear();
	FOR_EACH(s, styles) {
		if ((s->visible || s->visible.isScripted()) &&
		    nativeLook() || (
		      (s->width   || s->width  .isScripted()) &&
		      (s->height  || s->height .isScripted()))) {
			// no need to make a viewer for things that are always invisible
			viewers.push_back(makeViewer(s));
			// REMOVEME //TODO //%%%
			if (!viewers.back()) viewers.pop_back();
		}
	}
	// sort viewers by z-index of style
	stable_sort(viewers.begin(), viewers.end(), CompareViewer());
	onInit();
}

void DataViewer::setData(IndexMap<FieldP,ValueP>& values) {
	FOR_EACH(v, viewers) {
		v->setValue(values[v->getField()]);
	}
	onChange();
}


ValueViewerP DataViewer::makeViewer(const StyleP& style) {
	return style->makeViewer(*this, style);
}

void DataViewer::onAction(const Action&, bool undone) {
	// TODO
}
