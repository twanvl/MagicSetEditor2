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
#include <data/action/value.hpp>
#include <data/action/set.hpp>
#include <gui/util.hpp> // clearDC

DECLARE_TYPEOF_COLLECTION(ValueViewerP);
typedef IndexMap<FieldP,StyleP> IndexMap_FieldP_StyleP;
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_StyleP);

// ----------------------------------------------------------------------------- : DataViewer


// ----------------------------------------------------------------------------- : Drawing

void DataViewer::draw(DC& dc) {
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	RotatedDC rdc(dc, ss.card_angle(), stylesheet->getCardRect(), ss.card_zoom(), ss.card_anti_alias() && !nativeLook(), true);
	draw(rdc, stylesheet->card_background);
}
void DataViewer::draw(RotatedDC& dc, const Color& background) {
	if (!set) return; // no set specified, don't draw anything
	// fill with background color
	clearDC(dc.getDC(), background);
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

Rotation DataViewer::getRotation() const {
	if (!stylesheet) stylesheet = set->stylesheet;
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	return Rotation(ss.card_angle(), stylesheet->getCardRect(), ss.card_zoom(), true);
}

// ----------------------------------------------------------------------------- : Setting data

void DataViewer::setCard(const CardP& card) {
	if (!card) return; // TODO: clear editor?
	assert(set);
	this->card = card;
	stylesheet = set->stylesheetFor(card);
	setStyles(stylesheet, stylesheet->card_style);
	setData(card->data);
	onChangeSize();
}

// ----------------------------------------------------------------------------- : Viewers

struct CompareViewer {
	bool operator() (const ValueViewerP& a, const ValueViewerP& b) {
		return a->getStyle()->z_index < b->getStyle()->z_index;
	}
};

void DataViewer::setStyles(const StyleSheetP& stylesheet, IndexMap<FieldP,StyleP>& styles) {
	if (!viewers.empty() && styles.contains(viewers.front()->getStyle())) {
		// already using these styles
		return;
	}
	this->stylesheet = stylesheet;
	// create viewers
	viewers.clear();
	FOR_EACH(s, styles) {
		if ((s->visible || s->visible.isScripted()) &&
		    nativeLook() || (
		      (s->width   || s->width  .isScripted()) &&
		      (s->height  || s->height .isScripted()))) {
			// no need to make a viewer for things that are always invisible
			ValueViewerP viewer = makeViewer(s);
			if (viewer) viewers.push_back(viewer);
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

void DataViewer::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, DisplayChangeAction) {
		// refresh
		setCard(card);
		return;
	}
	TYPE_CASE(action, ValueAction) {
		FOR_EACH(v, viewers) {
			if (v->getValue() == action.valueP) {
				// refresh the viewer
				v->onAction(action, undone);
				onChange();
				return;
			}
		}
	}
	TYPE_CASE(action, ScriptValueEvent) {
		if (action.card == card.get()) {
			FOR_EACH(v, viewers) {
				if (v->getValue().get() == action.value) {
					// refresh the viewer
					v->onAction(action, undone);
					onChange();
					return;
				}
			}
		}
	}
	TYPE_CASE(action, ScriptStyleEvent) {
		if (action.stylesheet == stylesheet.get()) {
			FOR_EACH(v, viewers) {
				if (v->getStyle().get() == action.style) {
					// refresh the viewer
					v->onStyleChange();
					onChange();
					return;
				}
			}
		}
	}
}
