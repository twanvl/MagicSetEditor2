//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/text/element.hpp>
#include <data/symbol_font.hpp>

// ----------------------------------------------------------------------------- : SymbolTextElement

void SymbolTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	if (!(what & DRAW_NORMAL)) return;
	if (font.font) {
		font.font->draw(dc, ctx, rect, font.size * scale, font.alignment, content.substr(start - this->start, end-start));
	}
}

void SymbolTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	if (font.font) {
		font.font->getCharInfo(dc, ctx, font.size * scale, content.substr(start - this->start, end-start), out);
	}
}

double SymbolTextElement::minScale() const {
	return min(font.size(), font.scale_down_to) / max(0.01, font.size());
}
double SymbolTextElement::scaleStep() const {
	return 1. / max(font.size * 4, 1.);
}
