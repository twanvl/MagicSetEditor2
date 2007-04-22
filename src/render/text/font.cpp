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
	dc.SetFont(*font, scale);
	// draw shadow
	String text = content.substr(start - this->start, end - start);
	if (!text.empty() && text.GetChar(text.size() - 1) == _('\n')) {
		text = text.substr(0, text.size() - 1); // don't draw last \n
	}
	if (font->hasShadow()) {
		dc.SetTextForeground(font->shadow_color);
		dc.DrawText(text, rect.position() + font->shadow_displacement);
	}
	// draw
	dc.SetTextForeground(font->color);
	dc.DrawText(text, rect.position());
}

void FontTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	// font
	dc.SetFont(*font, scale);
	// find sizes & breaks
	double prev_width = 0;
	for (size_t i = start ; i < end ; ++i) {
		Char c = content.GetChar(i - this->start);
		RealSize s = dc.GetTextExtent(content.substr(start - this->start, i - start + 1));
		out.push_back(CharInfo(RealSize(s.width - prev_width, s.height),
						c == _('\n') ? break_style :
						c == _(' ')  ? BREAK_SOFT : BREAK_MAYBE
		             ));
		prev_width = s.width;
	}
}

double FontTextElement::minScale() const {
	return min(font->size(), font->scale_down_to) / max(0.01, font->size());
}
double FontTextElement::scaleStep() const {
	return 1. / max(font->size(), 1.);
}
