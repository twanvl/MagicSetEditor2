//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/element.hpp>

// ----------------------------------------------------------------------------- : CompoundTextElement

void CompoundTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	elements.draw(dc, scale, rect, xs, what, start, end);
}
void CompoundTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	elements.getCharInfo(dc, scale, start, end, out);
}
double CompoundTextElement::minScale() const {
	return elements.minScale();
}

// ----------------------------------------------------------------------------- : AtomTextElement

void AtomTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	if (what & DRAW_ACTIVE) {
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(Color(210,210,210));
		dc.DrawRectangle(rect);
	}
	CompoundTextElement::draw(dc, scale, rect, xs, what, start, end);
}
