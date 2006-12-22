/*//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/element.hpp>

// ----------------------------------------------------------------------------- : HorizontalLineTextElement

void HorizontalLineTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	// handled by TextViewer
}

void HorizontalLineTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	out.push_back(CharInfo(RealSize(0,0), BREAK_LINE));
}

double HorizontalLineTextElement::minScale() const {
	return 0; // we don't care about scaling
}
*/
