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
		dc.SetPen(viewer.borderPen(isCurrent()));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(styleP->getRect().grow(dc.trInvS(1)));
	}
}

bool ValueViewer::nativeLook() const {
	return viewer.nativeLook();
}
bool ValueViewer::isCurrent() const {
	return viewer.focusedViewer() == this;
}

// ----------------------------------------------------------------------------- : Type dispatch

#define IMPLEMENT_MAKE_VIEWER(Type)														\
	ValueViewerP Type##Style::makeViewer(DataViewer& parent, const StyleP& thisP) {		\
		assert(thisP.get() == this);													\
		return ValueViewerP(new Type##ValueViewer(parent, static_pointer_cast<Type##Style>(thisP)));	\
	}

IMPLEMENT_MAKE_VIEWER(Text);
IMPLEMENT_MAKE_VIEWER(Choice);
IMPLEMENT_MAKE_VIEWER(MultipleChoice);
IMPLEMENT_MAKE_VIEWER(Color);
IMPLEMENT_MAKE_VIEWER(Image);
IMPLEMENT_MAKE_VIEWER(Symbol);
