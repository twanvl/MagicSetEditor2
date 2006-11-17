//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/element.hpp>
#include <util/tagged_string.hpp>
#include <data/field/text.hpp>

DECLARE_TYPEOF_COLLECTION(TextElementP);

// ----------------------------------------------------------------------------- : TextElements

void TextElements::draw(RotatedDC& dc, double scale, const RealRect& rect, DrawWhat what, size_t start, size_t end) const {
	FOR_EACH_CONST(e, elements) {
		size_t start_ = max(start, e->start);
		size_t end_   = min(end,   e->end);
		if (start_ < end_) {
			e->draw(dc, scale, rect, what, start_, end_);
		}
		if (end <= e->end) return; // nothing can be after this
	}
}

void TextElements::getCharInfo(RotatedDC& dc, double scale, size_t start, size_t end, vector<CharInfo>& out) const {
	FOR_EACH_CONST(e, elements) {
		// characters before this element, after the previous
		for (size_t i = start ; i < e->start ; ++i) {
			out.push_back(CharInfo(RealSize(0,0), BREAK_NO));
		}
		e->getCharInfo(dc, scale, out);
		start = min(end, e->end);
	}
	for (size_t i = start ; i < end ; ++i) {
		out.push_back(CharInfo(RealSize(0,0), BREAK_NO));
	}
}

/*//@@
RealSize TextElements::charSize(const Rotation& rot, double scale, size_t index) const {
	vector<TextElementP>::const_iterator e = findByIndex(index);
	if (e != elements.end()) {
		return (*e)->charSize(rot, scale, index);
	} else {
		return RealSize(0,0);
	}
}

bool ends_before_index(const TextElementP& e, size_t index) {
	return index < e->end;
}

vector<TextElementP>::const_iterator TextElements::findByIndex(size_t index) const {
	// Note: slightly abusing lower_bound, since typeof(index) != elements.element_type
	vector<TextElementP>::const_iterator it = lower_bound(elements.begin(), elements.end(), index, ends_before_index);
	if ((*it)->start <= index && (*it)->end > index) return it;
	else                                             return elements.end();
}*/

// Helper class for TextElements::fromString, to allow persistent formating state accross recusive calls
struct TextElementsFromString {
	// What formatting is enabled?
	int bold, italic, symbol;
	int soft, kwpph;
	
	TextElementsFromString()
		: bold(0), italic(0), symbol(0), soft(0), kwpph(0) {}
	
	// read TextElements from a string
	void fromString(TextElements& te, const String& text, size_t start, size_t end, const TextStyle& style, Context& ctx) {
		te.elements.clear();
		// for each character...
		for (size_t pos = start ; pos < end ; ) {
			Char c = text.GetChar(pos);
			if (c == _('<')) {
				size_t tag_start = pos;
				pos = skip_tag(text, tag_start);
				if      (is_substr(text, tag_start, _( "<b")))          bold   += 1;
				else if (is_substr(text, tag_start, _("</b")))          bold   -= 1;
				else if (is_substr(text, tag_start, _( "<i")))          italic += 1;
				else if (is_substr(text, tag_start, _("</i")))          italic -= 1;
				else if (is_substr(text, tag_start, _( "<sym")))        symbol += 1;
				else if (is_substr(text, tag_start, _("</sym")))        symbol -= 1;
				else if (is_substr(text, tag_start, _( "<sep-soft")))   soft   += 1;
				else if (is_substr(text, tag_start, _("</sep-soft")))   soft   -= 1;
				else if (is_substr(text, tag_start, _( "<atom-kwpph"))) kwpph  += 1;
				else if (is_substr(text, tag_start, _("</atom-kwpph"))) kwpph  -= 1;
				else if (is_substr(text, tag_start, _("<line"))) {
					// horizontal line
					te.elements.push_back(new_shared3<HorizontalLineTextElement>(text, tag_start, pos));
/*				} else if (is_substr(text, start, _("<error"))) {
					// underline with wavy 'error' indicator
					size_t end = match_close_tag(text, tag_start);
					shared_ptr<ErrorTextElement> e(new ErrorTextElement(text, pos, end));
					fromString(e->elements, text, pos, end, style, ctx);
					pos = skip_tag(text, end);
				} else if (is_substr(text, start, _("<atom"))) {
					// 'atomic' indicator
					size_t end = match_close_tag(text, tag_start);
					shared_ptr<AtomTextElement> e(new AtomTextElement(text, pos, end));
					fromString(e->elements, text, pos, end, style, ctx);
					pos = skip_tag(text, end);
*/				} else {
					// ignore other tags
				}
			} else {
				// A character of normal text, add to the last text element (if possible)
				SimpleTextElement* e = nullptr;
				if (!te.elements.empty()) e = dynamic_cast<SimpleTextElement*>(te.elements.back().get());
				if (e && e->end == pos) {
					e->end = pos + 1; // just move the end, no need to make a new element
				} else {
					// add a new element for this text
					if (symbol > 0) {
						te.elements.push_back(new_shared3<SymbolTextElement>(text, pos, pos + 1));
					} else {
						te.elements.push_back(new_shared4<FontTextElement>  (text, pos, pos + 1, style.font.make(bold > 0, italic > 0)));
					}
				}
				pos += 1;
			}
		}
	}
};

void TextElements::fromString(const String& text, size_t start, size_t end, const TextStyle& style, Context& ctx) {
	TextElementsFromString f;
	f.fromString(*this, text, start, end, style, ctx);
}
/*
// ----------------------------------------------------------------------------- : CompoundTextElement

void CompoundTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, DrawWhat what, size_t start, size_t end) const {
	elements.draw(dc, scale, rect, what, start, end);
}
RealSize CompoundTextElement::charSize(RotatedDC& dc, double scale, size_t index) const {
	return elements.charSize(rot, scale, index);
}

*/