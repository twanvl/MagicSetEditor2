//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/text/element.hpp>
#include <util/tagged_string.hpp>
#include <data/field/text.hpp>
#include <gfx/color.hpp>

DECLARE_POINTER_TYPE(FontTextElement);

// ----------------------------------------------------------------------------- : TextElements

// Colors for <atom-param> tags
Color param_colors[] =
  { Color(0,170,0)
  , Color(0,0,200)
  , Color(200,0,100)
  , Color(200,200,0)
  , Color(0,170,170)
  , Color(200,0,0)
  };
const size_t param_colors_count = sizeof(param_colors) / sizeof(param_colors[0]);

// Helper class for TextElements::fromString, to allow persistent formating state accross recusive calls
struct TextElementsFromString {
  // What formatting is enabled?
  int bold = 0, italic = 0, symbol = 0;
  int soft = 0, kwpph = 0, param = 0, line = 0, soft_line = 0;
  int code = 0, code_kw = 0, code_string = 0, param_ref = 0;
  int param_id = 0;
  vector<Color> colors;
  vector<double> sizes;
  vector<String> fonts;
  vector<pair<double,double>> margins;
  vector<Alignment> aligns;

  const TextStyle& style;
  Context& ctx;
  vector<TextParagraph>& paragraphs;
  
  TextElementsFromString(TextElements& out, const String& text, const TextStyle& style, Context& ctx)
    : style(style), ctx(ctx), paragraphs(out.paragraphs)
  {
    out.start = 0;
    out.end = text.size();
    paragraphs.emplace_back();
    paragraphs.back().start = 0;
    fromString(out.children, text, 0, text.size());
    paragraphs.back().end = text.size();
  }

private:
  // read TextElements from a string
  void fromString(vector<TextElementP>& elements, const String& text, size_t start, size_t end) {
    size_t text_start = start;
    // for each character...
    for (size_t pos = start ; pos < end ; ) {
      Char c = text.GetChar(pos);
      if (c == _('<')) {
        if (text_start < pos) {
          // text element before this tag?
          addText(elements, text, text_start, pos);
          addParagraphs(text, text_start, pos);
        }
        // a (formatting) tag
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
            auto c = parse_color(text.substr(colon+1, pos-colon-2));
            if (c) {
              colors.push_back(*c);
            } else {
              queue_message(MESSAGE_WARNING, _("Invalid color in tagged string: ") + text.substr(colon + 1, pos - colon - 2));
              colors.push_back(style.font.color);
            }
          }
        } else if (is_substr(text, tag_start, _("</color"))) {
          if (!colors.empty()) colors.pop_back();
        }
        else if (is_substr(text, tag_start, _( "<font"))) {
          size_t colon = text.find_first_of(_(">:"), tag_start);
          if (colon < pos - 1 && text.GetChar(colon) == _(':')) {
            fonts.push_back(text.substr(colon+1, pos-colon-2));
          }
        } else if (is_substr(text, tag_start, _("</font"))) {
          if (!fonts.empty()) fonts.pop_back();
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
          #if 0
            // it would be nice if we could have semi-transparent brushes
            Color color = style.font.color; color.a /= 5;
          #else
            Color fg = style.font.color;
            Color color = fg.r+fg.g+fg.b < 255*2 ? Color(210,210,210) : Color(60,60,60);
          #endif
          size_t end_tag = min(end, match_close_tag(text, tag_start));
          intrusive_ptr<AtomTextElement> e = make_intrusive<AtomTextElement>(pos, end_tag, color);
          fromString(e->children, text, pos, end_tag);
          elements.push_back(e);
          pos = skip_tag(text, end_tag);
        } else if (is_substr(text, tag_start, _( "<error"))) {
          // error indicator
          size_t end_tag = min(end, match_close_tag(text, tag_start));
          intrusive_ptr<ErrorTextElement> e = make_intrusive<ErrorTextElement>(pos, end_tag);
          fromString(e->children, text, pos, end_tag);
          elements.push_back(e);
          pos = skip_tag(text, end_tag);
        } else if (is_substr(text, tag_start, _("</li"))) {
          // end of bullet point, set margin here
          paragraphs.back().margin_end_char = pos;
        } else if (is_substr(text, tag_start, _("<margin"))) {
          size_t colon = text.find_first_of(_(">:"), tag_start);
          if (colon < pos - 1 && text.GetChar(colon) == _(':')) {
            size_t colon2 = text.find_first_of(_(">:"), colon+1);
            double margin_left = 0., margin_right = 0.;
            text.substr(colon + 1, colon2 - colon - 2).ToDouble(&margin_left);
            text.substr(colon2 + 1, pos - colon2 - 2).ToDouble(&margin_right);
            if (!margins.empty()) {
              margin_left += margins.back().first;
              margin_right += margins.back().second;
            }
            margins.emplace_back(margin_left, margin_right);
            paragraphs.back().margin_left = margin_left;
            paragraphs.back().margin_right = margin_right;
          }
        } else if (is_substr(text, tag_start, _("</margin"))) {
          if (!margins.empty()) margins.pop_back();
        } else if (is_substr(text, tag_start, _("<align"))) {
          size_t colon = text.find_first_of(_(">:"), tag_start);
          if (colon < pos - 1 && text.GetChar(colon) == _(':')) {
            Alignment align = alignment_from_string(text.substr(colon+1, pos-colon-2));
            aligns.push_back(align);
            paragraphs.back().alignment = align;
          }
        } else if (is_substr(text, tag_start, _("</align"))) {
          if (!aligns.empty()) aligns.pop_back();
        } else {
          // ignore other tags
        }
        // text starts again after the tag
        text_start = pos;
      } else {
        // normal text
        pos += 1;
      }
    }
    if (text_start < end) {
      // remaining text at the end
      addText(elements, text, text_start, end);
      addParagraphs(text, text_start, end);
    }
  }
  
private:
  /// Create a text element for a piece of text, text[start..end)
  void addText(vector<TextElementP>& elements, const String& text, size_t start, size_t end) {
    String content = untag(text.substr(start, end - start));
    assert(content.size() == end-start);
    // use symbol font?
    if (symbol > 0 && style.symbol_font.valid()) {
      elements.push_back(make_intrusive<SymbolTextElement>(content, start, end, style.symbol_font, &ctx));
    } else {
      // text, possibly mixed with symbols
      DrawWhat what = soft > 0 ? DRAW_ACTIVE : DRAW_NORMAL;
      LineBreak line_break = line > 0 ? LineBreak::LINE :
                             soft_line > 0 ? LineBreak::SOFT : LineBreak::HARD;
      if (kwpph > 0 || param > 0) {
        // bracket the text
        content = String(LEFT_ANGLE_BRACKET) + content + RIGHT_ANGLE_BRACKET;
        start -= 1;
        end   += 1;
      } else if (style.always_symbol && style.symbol_font.valid()) {
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
              elements.push_back(make_intrusive<FontTextElement>(content.substr(text_pos, pos-text_pos), start+text_pos, start+pos, font, what, line_break));
            }
            elements.push_back(make_intrusive<SymbolTextElement>(content.substr(pos,n), start+pos, start+pos+n, style.symbol_font, &ctx));
            text_pos = pos += n;
          } else {
            ++pos;
          }
        }
        if (text_pos < pos) {
          if (!font) font = makeFont(style);
          elements.push_back(make_intrusive<FontTextElement>(content.substr(text_pos), start+text_pos, end, font, what, line_break));
        }
      } else {
        elements.push_back(make_intrusive<FontTextElement>(content, start, end, makeFont(style), what, line_break));
      }
    }
  }
  // Find paragraph breaks in text
  void addParagraphs(const String& text, size_t start, size_t end) {
    if (line == 0 && soft_line > 0) return;
    for (size_t i = start; i < end; ++i) {
      wxUniChar c = text.GetChar(i);
      if (c == '\n') {
        paragraphs.back().end = i + 1;
        paragraphs.emplace_back();
        paragraphs.back().start = i + 1;
        paragraphs.back().margin_end_char = i + 1;
        if (!margins.empty()) {
          paragraphs.back().margin_left  = margins.back().first;
          paragraphs.back().margin_right = margins.back().second;
        }
        if (!aligns.empty()) {
          paragraphs.back().alignment = aligns.back();
        }
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
      fonts.empty() ? nullptr : &fonts.back(),
      param > 0 || param_ref > 0
        ? &param_colors[(param_id++) % param_colors_count]
      : !colors.empty()
        ? &colors.back()
        : nullptr,
      !sizes.empty() ? &sizes.back() : nullptr
      );
  }
};

void TextElements::clear() {
  children.clear();
  paragraphs.clear();
}

void TextElements::fromString(const String& text, const TextStyle& style, Context& ctx) {
  clear();
  TextElementsFromString f(*this, text, style, ctx);
}
