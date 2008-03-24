//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
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
			out.push_back(CharInfo());
		}
		e->getCharInfo(dc, scale, out);
	}
	while (out.size() < end) {
		out.push_back(CharInfo());
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
	int soft, kwpph, param, line, soft_line;
	int code, code_kw, code_string, param_ref, error;
	int param_id;
	vector<Color>  colors;
	vector<double> sizes;
	/// put angle brackets around the text?
	bool bracket;
	
	TextElementsFromString()
		: bold(0), italic(0), symbol(0), soft(0), kwpph(0), param(0), line(0), soft_line(0)
		, code(0), code_kw(0), code_string(0), param_ref(0), error(0)
		, param_id(0), bracket(false) {}
	
	// read TextElements from a string
	void fromString(TextElements& te, const String& text, size_t start, size_t end, const TextStyle& style, Context& ctx) {
		te.elements.clear();
		end = min(end, text.size());
		size_t text_start = start;
		// for each character...
		for (size_t pos = start ; pos < end ; ) {
			Char c = text.GetChar(pos);
			if (c == _('<')) {
				if (text_start < pos) {
					// text element before this tag?
					addText(te, text, text_start, pos, style, ctx);
				}
				size_t tag_start = pos;
				pos = skip_tag(text, tag_start);
				if      (is_substr(text, tag_start, _( "<b")))          bold        += 1;
				else if (is_substr(text, tag_start, _("</b")))          bold        -= 1;
				else if (is_substr(text, tag_start, _( "<i")))          italic      += 1;
				else if (is_substr(text, tag_start, _("</i")))          italic      -= 1;
				else if (is_substr(text, tag_start, _( "<sym")))        symbol      += 1;
				else if (is_substr(text, tag_start, _("</sym")))        symbol      -= 1;
				else if (is_substr(text, tag_start, _( "<line")))       line        += 1;
				else if (is_substr(text, tag_start, _("</line")))       line        -= 1;
				else if (is_substr(text, tag_start, _( "<soft-line")))  soft_line   += 1;
				else if (is_substr(text, tag_start, _("</soft-line")))  soft_line   -= 1;
				else if (is_substr(text, tag_start, _( "<sep-soft")))   soft        += 1;
				else if (is_substr(text, tag_start, _("</sep-soft")))   soft        -= 1;
				else if (is_substr(text, tag_start, _( "<soft")))       soft        += 1; // must be after <soft-line
				else if (is_substr(text, tag_start, _("</soft")))       soft        -= 1;
				else if (is_substr(text, tag_start, _( "<atom-kwpph"))) kwpph       += 1;
				else if (is_substr(text, tag_start, _("</atom-kwpph"))) kwpph       -= 1;
				else if (is_substr(text, tag_start, _( "<code-kw")))    code_kw     += 1;
				else if (is_substr(text, tag_start, _("</code-kw")))    code_kw     -= 1;
				else if (is_substr(text, tag_start, _( "<code-str")))   code_string += 1;
				else if (is_substr(text, tag_start, _("</code-str")))   code_string -= 1;
				else if (is_substr(text, tag_start, _( "<code")))       code        += 1;
				else if (is_substr(text, tag_start, _("</code")))       code        -= 1;
				else if (is_substr(text, tag_start, _( "<color"))) {
					size_t colon = text.find_first_of(_(">:"), tag_start);
					if (colon < pos - 1 && text.GetChar(colon) == _(':')) {
						Color c = parse_color(text.substr(colon+1, pos-colon-2));
						if (!c.Ok()) c = style.font.color;
						colors.push_back(c);
					}
				} else if (is_substr(text, tag_start, _("</color"))) {
					if (!colors.empty()) colors.pop_back();
				}
				else if (is_substr(text, tag_start, _( "<size"))) {
					size_t colon = text.find_first_of(_(">:"), tag_start);
					if (colon < pos - 1 && text.GetChar(colon) == _(':')) {
						double size = style.font.size;
						String v = text.substr(colon+1, pos-colon-2);
						v.ToDouble(&size);
						sizes.push_back(size);
					}
				} else if (is_substr(text, tag_start, _("</size"))) {
					if (!sizes.empty()) sizes.pop_back();
				}
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
				else if (is_substr(text, tag_start, _("<atom"))) {
					// 'atomic' indicator
					size_t end_tag = min(end, match_close_tag(text, tag_start));
					intrusive_ptr<AtomTextElement> e(new AtomTextElement(pos, end_tag));
					fromString(e->elements, text, pos, end_tag, style, ctx);
					te.elements.push_back(e);
					pos = skip_tag(text, end_tag);
				} else if (is_substr(text, tag_start, _( "<error"))) {
					// error indicator
					size_t end_tag = min(end, match_close_tag(text, tag_start));
					intrusive_ptr<ErrorTextElement> e(new ErrorTextElement(pos, end_tag));
					fromString(e->elements, text, pos, end_tag, style, ctx);
					te.elements.push_back(e);
					pos = skip_tag(text, end_tag);
				} else {
					// ignore other tags
				}
				text_start = pos;
			} else {
				pos += 1;
			}
		}
		if (text_start < end) {
			addText(te, text, text_start, end, style, ctx);
		}
	}
	
  private:
	/// Create a text element for a piece of text
	void addText(TextElements& te, const String& text, size_t start, size_t end, const TextStyle& style, Context& ctx) {
		String content = untag(text.substr(start, end - start));
		assert(content.size() == end-start);
		// use symbol font?
		if (symbol > 0 && style.symbol_font.valid()) {
			te.elements.push_back(new_intrusive5<SymbolTextElement>(content, start, end, style.symbol_font, &ctx));
		} else {
			// text, possibly mixed with symbols
			DrawWhat what = soft > 0 ? DRAW_ACTIVE : DRAW_NORMAL;
			LineBreak line_break = line > 0 ? BREAK_LINE :
			                       soft_line > 0 ? BREAK_SOFT : BREAK_HARD;
			if (kwpph > 0 || param > 0) {
				// bracket the text
				content = String(LEFT_ANGLE_BRACKET) + content + RIGHT_ANGLE_BRACKET;
				start -= 1;
				end   += 1;
			}
			if (style.always_symbol && style.symbol_font.valid()) {
				// mixed symbols/text, autodetected by symbol font
				size_t text_pos = 0;
				size_t pos = 0;
				FontP font;
				while (pos < end-start) {
					if (size_t n = style.symbol_font.font->recognizePrefix(content,pos)) {
						// at 'pos' there are n symbol font characters
						if (text_pos < pos) {
							// text before it?
							if (!font) font = makeFont(style);
							te.elements.push_back(new_intrusive6<FontTextElement>(content.substr(text_pos, pos-text_pos), start+text_pos, start+pos, font, what, line_break));
						}
						te.elements.push_back(new_intrusive5<SymbolTextElement>(content.substr(pos,n), start+pos, start+pos+n, style.symbol_font, &ctx));
						text_pos = pos += n;
					} else {
						++pos;
					}
				}
				if (text_pos < pos) {
					if (!font) font = makeFont(style);
					te.elements.push_back(new_intrusive6<FontTextElement>(content.substr(text_pos), start+text_pos, end, font, what, line_break));
				}
			} else {
				te.elements.push_back(new_intrusive6<FontTextElement>(content, start, end, makeFont(style), what, line_break));
			}
		}
	}
	
	FontP makeFont(const TextStyle& style) {
		return style.font.make(
			(bold        > 0 ? FONT_BOLD        : FONT_NORMAL) |
			(italic      > 0 ? FONT_ITALIC      : FONT_NORMAL) |
			(soft        > 0 ? FONT_SOFT        : FONT_NORMAL) |
			(kwpph       > 0 ? FONT_SOFT        : FONT_NORMAL) |
			(code        > 0 ? FONT_CODE        : FONT_NORMAL) |
			(code_kw     > 0 ? FONT_CODE_KW     : FONT_NORMAL) |
			(code_string > 0 ? FONT_CODE_STRING : FONT_NORMAL),
			param > 0 || param_ref > 0
				? &param_colors[(param_id++) % param_colors_count]
			: !colors.empty()
				? &colors.back()
				: nullptr,
			!sizes.empty() ? &sizes.back() : nullptr
			);
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
