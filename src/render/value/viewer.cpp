//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/viewer.hpp>
#include <render/value/text.hpp>
#include <render/value/choice.hpp>
#include <render/value/multiple_choice.hpp>
#include <render/value/boolean.hpp>
#include <render/value/image.hpp>
#include <render/value/symbol.hpp>
#include <render/value/color.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : ValueViewer

ValueViewer::ValueViewer(DataViewer& parent, const StyleP& style)
	: viewer(parent), styleP(style)
{}

Set& ValueViewer::getSet() const { return *viewer.getSet(); }

void ValueViewer::setValue(const ValueP& value) {
	assert(value->fieldP == styleP->fieldP); // matching field
	valueP = value;
	onValueChange();
}

bool ValueViewer::containsPoint(const RealPoint& p) const {
	return p.x >= styleP->left
	    && p.y >= styleP->top
	    && p.x <  styleP->left + (int)(styleP->width)
	    && p.y <  styleP->top  + (int)(styleP->height);
}
RealRect ValueViewer::boundingBox() const {
	return styleP->getRect().grow(1);
}

void ValueViewer::drawFieldBorder(RotatedDC& dc) {
	if (viewer.drawBorders() && getField()->editable) {
		dc.SetPen(viewer.borderPen(viewer.focusedViewer() == this));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(styleP->getRect().grow(dc.trInvS(1)));
	}
}

// ----------------------------------------------------------------------------- : Development/debug

#ifdef _DEBUG

// REMOVEME

#include <data/field.hpp>
#include <data/field/choice.hpp>
#include <data/field/boolean.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/field/color.hpp>
#include <data/field/image.hpp>
#include <data/field/symbol.hpp>
#include <data/field/text.hpp>

#define IMPLEMENT_MAKE_VIEWER(Type)														\
	ValueViewerP Type##Style::makeViewer(DataViewer& parent, const StyleP& thisP) {		\
		assert(thisP.get() == this);													\
		return ValueViewerP(new Type##ValueViewer(parent, static_pointer_cast<Type##Style>(thisP)));	\
	}

ValueViewerP ChoiceStyle        ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP BooleanStyle       ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP MultipleChoiceStyle::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP ColorStyle         ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
//ValueViewerP ImageStyle         ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
IMPLEMENT_MAKE_VIEWER(Image);
ValueViewerP SymbolStyle        ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP TextStyle          ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }

ValueEditorP ChoiceStyle        ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP BooleanStyle       ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP MultipleChoiceStyle::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP ColorStyle         ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP ImageStyle         ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP SymbolStyle        ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP TextStyle          ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }

#endif