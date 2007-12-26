//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/element.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : FontTextElement

void FontTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	if ((what & draw_as) != draw_as) return; // don't draw
	// text
	String text = content.substr(start - this->start, end - start);
	if (!text.empty() && text.GetChar(text.size() - 1) == _('\n')) {
		text = text.substr(0, text.size() - 1); // don't draw last \n
	}
	// draw
	dc.SetFont(*font, scale);
	dc.DrawTextWithShadow(text, *font, rect.position());
}

void FontTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	// font
	dc.SetFont(*font, scale);
	// find sizes & breaks
	double prev_width = 0;
	size_t line_start = start; // start of the current line
	for (size_t i = start ; i < end ; ++i) {
		Char c = content.GetChar(i - this->start);
		if (c == _('\n')) {
			out.push_back(CharInfo(RealSize(0, dc.GetCharHeight()), break_style, draw_as == DRAW_ACTIVE));
			line_start = i + 1;
			prev_width = 0;
		} else {
			RealSize s = dc.GetTextExtent(content.substr(line_start - this->start, i - line_start + 1));
			out.push_back(CharInfo(
			                 RealSize(s.width - prev_width, s.height),
			                 c == _(' ') ? BREAK_SPACE : BREAK_MAYBE,
			                 draw_as == DRAW_ACTIVE // from <soft> tag
			             ));
			prev_width = s.width;
		}
	}
}

double FontTextElement::minScale() const {
	return min(font->size(), font->scale_down_to) / max(0.01, font->size());
}
double FontTextElement::scaleStep() const {
	return 1. / max(font->size() * 4, 1.);
}
