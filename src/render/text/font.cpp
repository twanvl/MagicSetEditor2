//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/element.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : FontTextElement

void FontTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, DrawWhat what, size_t start, size_t end) const {
	dc.SetFont(font->font, font->size * scale);
	
	if (end != start && text.substr(end-1, 1) == _("\n")) end -= 1; // don't draw the newline character at the end
/*	if ((draw & DRAW_NORMAL) != DRAW_NORMAL) {
		// don't draw
	if (what == DRAW_ACTIVE) {
		// we are drawing a separator
		dc.SetTextForeground(font->separator_color);
		dc.DrawText(text.substr(start, end-start), rect.position);
	}
	} else {*/
		// draw normally
		// draw shadow
		if (font->hasShadow()) {
			dc.SetTextForeground(font->shadow_color);
			dc.DrawText(text.substr(start, end - start), rect.position + font->shadow_displacement);
		}
		// draw
		dc.SetTextForeground(font->color);
		dc.DrawText(text.substr(start, end - start), rect.position);
//	}
}

void FontTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
	// font
	dc.SetFont(font->font, font->size * scale);
	// find sizes & breaks
	double prev_width = 0;
	for (size_t i = start ; i < end ; ++i) {
		Char c = text.GetChar(i);
		RealSize s = dc.GetTextExtent(text.substr(start, i - start));
		out.push_back(CharInfo(RealSize(s.width - prev_width, s.height),
						c == _('\n') ? BREAK_HARD :
						c == _(' ')  ? BREAK_SOFT : BREAK_NO
		             ));
		prev_width = s.width;
	}
}
