//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
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
DECLARE_TYPEOF_NO_REV(IndexMap<FieldP COMMA StyleP>);

// ----------------------------------------------------------------------------- : DataViewer

DataViewer::DataViewer() : drawing(false) {}
DataViewer::~DataViewer() {}

// ----------------------------------------------------------------------------- : Drawing

void DataViewer::draw(DC& dc) {
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	RotatedDC rdc(dc, getRotation(),
	              nativeLook() ? QUALITY_LOW : (ss.card_anti_alias() ? QUALITY_AA : QUALITY_SUB_PIXEL));
	draw(rdc, stylesheet->card_background);
}
void DataViewer::draw(RotatedDC& dc, const Color& background) {
	if (!set) return; // no set specified, don't draw anything
	drawing = true;
	// fill with background color
	clearDC(dc.getDC(), background);
	// update style scripts
	updateStyles(false);
	// prepare viewers
	bool changed_content_properties = false;
	FOR_EACH(v, viewers) { // draw low z index fields first
		if (v->getStyle()->isVisible()) {
			Rotater r(dc, v->getRotation());
			try {
				if (v->prepare(dc)) {
					changed_content_properties = true;
				}
			} catch (const Error& e) {
				handle_error(e, false, false);
			}
		}
	}
	if (changed_content_properties) {
		updateStyles(true);
	}
	// draw viewers
	FOR_EACH(v, viewers) { // draw low z index fields first
		if (v->getStyle()->isVisible()) {// visible
			Rotater r(dc, v->getRotation());
			try {
				drawViewer(dc, *v);
			} catch (const Error& e) {
				handle_error(e, false, false);
			}
		}
	}
	drawing = false;
}
void DataViewer::drawViewer(RotatedDC& dc, ValueViewer& v) {
	v.draw(dc);
}

void DataViewer::updateStyles(bool only_content_dependent) {
	try {
		if (card) {
			set->updateStyles(card, only_content_dependent);
		} else {
			Context& ctx = getContext();
			FOR_EACH(v, viewers) {
				Style& s = *v->getStyle();
				if (only_content_dependent && !s.content_dependent) continue;
				if (s.update(ctx)) {
					s.tellListeners(only_content_dependent);
				}
			}
		}
	} catch (const Error& e) {
		handle_error(e, false, false);
	}
}

// ----------------------------------------------------------------------------- : Utility for ValueViewers

bool   DataViewer::nativeLook()    const { return false; }
bool   DataViewer::drawBorders()   const { return false; }
bool   DataViewer::drawEditing()   const { return false; }
bool   DataViewer::drawFocus()     const { return false; }
wxPen  DataViewer::borderPen(bool) const { return wxPen(); }
ValueViewer* DataViewer::focusedViewer() const { return nullptr; }
Context& DataViewer::getContext()  const { return set->getContext(card); }

Rotation DataViewer::getRotation() const {
	if (!stylesheet) stylesheet = set->stylesheet;
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	return Rotation(ss.card_angle(), stylesheet->getCardRect(), ss.card_zoom(), 1.0, ROTATION_ATTACH_TOP_LEFT);
}

Package& DataViewer::getStylePackage() const {
	return *stylesheet;
}
Package& DataViewer::getLocalPackage() const {
	return *set;
}
Game& DataViewer::getGame() const {
	return *set->game;
}

// ----------------------------------------------------------------------------- : Setting data

void DataViewer::setCard(const CardP& card, bool refresh) {
	if (!card) return; // TODO: clear viewer?
	StyleSheetP new_stylesheet = set->stylesheetForP(card);
	if (!refresh && this->card == card && this->stylesheet == new_stylesheet) return; // already set
	assert(set);
	this->card = card;
	stylesheet = new_stylesheet;
	setStyles(stylesheet, stylesheet->card_style, &stylesheet->extra_card_style);
	setData(card->data, &card->extraDataFor(*stylesheet));
	onChangeSize();
}

void DataViewer::onChangeSet() {
	viewers.clear();
	onInit();
	onChange();
	onChangeSize();
}

// ----------------------------------------------------------------------------- : Viewers

struct CompareViewer {
	bool operator() (const ValueViewerP& a, const ValueViewerP& b) {
		return a->getStyle()->z_index < b->getStyle()->z_index;
	}
};

void DataViewer::setStyles(const StyleSheetP& stylesheet, IndexMap<FieldP,StyleP>& styles, IndexMap<FieldP,StyleP>* extra_styles) {
	if (!viewers.empty() && styles.contains(viewers.front()->getStyle())) {
		// already using these styles
		return;
	}
	this->stylesheet = stylesheet;
	// create viewers
	viewers.clear();
	addStyles(styles);
	if (extra_styles) addStyles(*extra_styles);
	// sort viewers by z-index of style
	stable_sort(viewers.begin(), viewers.end(), CompareViewer());
	onInit();
}
void DataViewer::addStyles(IndexMap<FieldP,StyleP>& styles) {
	FOR_EACH(s, styles) {
		if ((s->visible || s->visible.isScripted()) && (nativeLook() || s->hasSize())) {
			// no need to make a viewer for things that are always invisible
			ValueViewerP viewer = makeViewer(s);
			if (viewer) viewers.push_back(viewer);
		}
	}
}

void DataViewer::setData(IndexMap<FieldP,ValueP>& values, IndexMap<FieldP,ValueP>* extra_values) {
	FOR_EACH(v, viewers) {
		// is this field contained in values?
		ValueP val = values.tryGet(v->getField());
		if (val) {
			v->setValue(val);
		} else {
			// if it is not in values it should be in extra values
			assert(extra_values);
			val = extra_values->tryGet(v->getField());
			assert(val);
			v->setValue(val);
		}
	}
	onChange();
}


ValueViewerP DataViewer::makeViewer(const StyleP& style) {
	return style->makeViewer(*this, style);
}

void DataViewer::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, DisplayChangeAction) {
		// refresh
		setCard(card, true);
		return;
	}
	TYPE_CASE(action, ValueAction) {
		if (action.card == card.get()) {
			FOR_EACH(v, viewers) {
				if (v->getValue()->equals( action.valueP.get() )) {
					// refresh the viewer
					v->onAction(action, undone);
					onChange();
					return;
				}
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
/*//%	TYPE_CASE(action, ScriptStyleEvent) {
		if (action.stylesheet == stylesheet.get()) {
			FOR_EACH(v, viewers) {
				if (v->getStyle().get() == action.style) {
					// refresh the viewer
					v->onStyleChange();
					if (!drawing) onChange();
					return;
				}
			}
		}
	}*/
}
