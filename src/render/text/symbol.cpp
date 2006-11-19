//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/element.hpp>

// ----------------------------------------------------------------------------- : SymbolTextElement

void SymbolTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, double* xs, DrawWhat what, size_t start, size_t end) const {
	// TODO
}

void SymbolTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	// TODO
}

double SymbolTextElement::minScale() const {
	return 1; // TODO
}
