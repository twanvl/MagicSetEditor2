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
DECLARE_POINTER_TYPE(FontTextElement);

// ----------------------------------------------------------------------------- : TextElements

void TextElements::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
	FOR_EACH_CONST(e, elements) {
		size_t start_ = max(start, e->start);
		size_t end_   = min(end,   e->end);
		if (start_ < end_) {
			e->draw(dc, scale,
			        RealRect(rect.x + xs[start_-start] - xs[0], rect.y,
			                 xs[end_-start] - xs[start_-start], rect.height),
			        xs + start_ - start, what, start_, end_);
		}
		if (end <= e->end) return; // nothing can be after this
	}
}

void TextElements::getCharInfo(RotatedDC& dc, double scale, size_t start, size_t end, vector<CharInfo>& out) const {
	FOR_EACH_CONST(e, elements) {
		// characters before this element, after the previous
		while (out.size() < e->start) {
			out.push_back(CharInfo(RealSize(0,0), BREAK_NO));
		}
		e->getCharInfo(dc, scale, out);
	}
	while (out.size() < end) {
		out.push_back(CharInfo(RealSize(0,0), BREAK_NO));
	}
}

double TextElements::minScale() const {
	double m = 0.0001;
	FOR_EACH_CONST(e, elements) {
		m = max(m, e->minScale());
	}
	return m;
}
double TextElements::scaleStep() const {
	double m = 1;
	FOR_EACH_CONST(e, elements) {
		m = min(m, e->scaleStep());
	}
	return m;
}

// Colors for <atom-param> tags
Color param_colors[] =
	{	Color(0,170,0)
	,	Color(0,0,200)
	,	Color(200,0,100)
	,	Color(200,200,0)
	,	Color(0,170,170)
	,	Color(200,0,0)
	};
const size_t param_colors_count = sizeof(param_colors) / sizeof(param_colors[0]);

// Helper class for TextElements::fromString, to allow persistent formating state accross recusive calls
struct TextElementsFromString {
	// What formatting is enabled?
	int bold, italic, symbol;
	int soft, kwpph, param, line;
	int code, code_kw, code_string, param_ref, error;
	int param_id;
	bool bracket;
	
	TextElementsFromString()
		: bold(0), italic(0), symbol(0), soft(0), kwpph(0), param(0), line(0)
		, code(0), code_kw(0), code_string(0), param_ref(0), error(0)
		, param_id(0), bracket(false) {}
	
	// read TextElements from a string
	void fromString(TextElements& te, const String& text, size_t start, size_t end, const TextStyle& style, Context& ctx) {
		te.elements.clear();
		end = min(end, text.size());
		// for each character...
		for (size_t pos = start ; pos < end ; ) {
			Char c = text.GetChar(pos);
			if (c == _('<')) {
				size_t tag_start = pos;
				pos = skip_tag(text, tag_start);
				if      (is_substr(text, tag_start, _( "<b")))          bold        += 1;
				else if (is_substr(text, tag_start, _("</b")))          bold        -= 1;
				else if (is_substr(text, tag_start, _( "<i")))          italic      += 1;
				else if (is_substr(text, tag_start, _("</i")))          italic      -= 1;
				else if (is_substr(text, tag_start, _( "<sym")))        symbol      += 1;
				else if (is_substr(text, tag_start, _("</sym")))        symbol      -= 1;
				else if (is_substr(text, tag_start, _( "<sep-soft")))   soft        += 1;
				else if (is_substr(text, tag_start, _("</sep-soft")))   soft        -= 1;
				else if (is_substr(text, tag_start, _( "<atom-kwpph"))) kwpph       += 1;
				else if (is_substr(text, tag_start, _("</atom-kwpph"))) kwpph       -= 1;
				else if (is_substr(text, tag_start, _( "<code-kw")))    code_kw     += 1;
				else if (is_substr(text, tag_start, _("</code-kw")))    code_kw     -= 1;
				else if (is_substr(text, tag_start, _( "<code-str")))   code_string += 1;
				else if (is_substr(text, tag_start, _("</code-str")))   code_string -= 1;
				else if (is_substr(text, tag_start, _( "<code")))       code        += 1;
				else if (is_substr(text, tag_start, _("</code")))       code        -= 1;
				else if (is_substr(text, tag_start, _( "<ref-param"))) {
					// determine the param being referenced
					// from a tag <ref-param123>
					if (pos != String::npos) {
						String ref = text.substr(tag_start + 10, pos - tag_start - 11);
						long ref_n;
						if (ref.ToLong(&ref_n)) {
							param_id = (ref_n - 1)%param_colors_count + param_colors_count;
						}
					}
					param_ref += 1;
				}
				else if (is_substr(text, tag_start, _("</ref-param")))  param_ref   -= 1;
				else if (is_substr(text, tag_start, _( "<atom-param"))) param       += 1;
				else if (is_substr(text, tag_start, _("</atom-param"))) param       -= 1;
				else if (is_substr(text, tag_start, _( "<line")))       line        += 1;
				else if (is_substr(text, tag_start, _("</line")))       line        -= 1;
				else if (is_substr(text, tag_start, _("<atom"))) {
					// 'atomic' indicator
					size_t end_tag = min(end, match_close_tag(text, tag_start));
					shared_ptr<AtomTextElement> e(new AtomTextElement(text, pos, end_tag));
					fromString(e->elements, text, pos, end_tag, style, ctx);
					te.elements.push_back(e);
					pos = skip_tag(text, end_tag);
				} else if (is_substr(text, tag_start, _( "<error"))) {
					// error indicator
					size_t end_tag = min(end, match_close_tag(text, tag_start));
					shared_ptr<ErrorTextElement> e(new ErrorTextElement(text, pos, end_tag));
					fromString(e->elements, text, pos, end_tag, style, ctx);
					te.elements.push_back(e);
					pos = skip_tag(text, end_tag);
				} else {
					// ignore other tags
				}
			} else {
				if (c == _('\1')) c = _('<'); // unescape
				// A character of normal text, add to the last text element (if possible)
				SimpleTextElement* e = nullptr;
				if (!te.elements.empty()) e = dynamic_cast<SimpleTextElement*>(te.elements.back().get());
				if (e && e->end == (bracket ? pos + 1 : pos)) {
					e->end = bracket ? pos + 2 : pos + 1; // just move the end, no need to make a new element
					e->content += c;
					if (bracket) {
						// content is "<somethin>g" should be "<something>"
						swap(e->content[e->content.size() - 2], e->content[e->content.size() - 1]);
					}
				} else {
					// add a new element for this text
					if (symbol > 0 && style.symbol_font.valid()) {
						e = new SymbolTextElement(text, pos, pos + 1, style.symbol_font, &ctx);
						bracket = false;
					} else {
						FontP font = style.font.make(bold > 0, italic > 0, soft > 0 || kwpph > 0, code > 0,
						                             param > 0 || param_ref > 0
						                               ? &param_colors[(param_id++) % param_colors_count]
						                               : nullptr);
						bracket = kwpph > 0 || param > 0;
						e = new FontTextElement(
									text,
									bracket ? pos - 1 : pos,
									bracket ? pos + 2 : pos + 1,
									font,
									soft > 0 ? DRAW_ACTIVE : DRAW_NORMAL,
									line > 0 ? BREAK_LINE : BREAK_HARD);
					}
					if (bracket) {
						e->content = String(_("‹")) + c + _("›");
					} else {
						e->content = c;
					}
					te.elements.push_back(TextElementP(e));
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
