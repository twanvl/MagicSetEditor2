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
double CompoundTextElement::scaleStep() const {
	return elements.scaleStep();
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

// ----------------------------------------------------------------------------- : ErrorTextElement

void ErrorTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	// Draw wavy underline
	dc.SetPen(*wxRED_PEN);
	RealPoint pos = rect.bottomLeft() - dc.trInvS(RealSize(0,2));
	RealSize  dx(dc.trInvS(2), 0), dy(0, dc.trInvS(1));
	while (pos.x + 1 < rect.right()) {
		dc.DrawLine(pos - dy, pos + dx + dy);
		pos += dx;
		dy  = -dy;
	}
	if (pos.x < rect.right()) {
		// final piece
		dc.DrawLine(pos - dy, pos + dx / 2);
	}
	// Draw the contents
	CompoundTextElement::draw(dc, scale, rect, xs, what, start, end);
}