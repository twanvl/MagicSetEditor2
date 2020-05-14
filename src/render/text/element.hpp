//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <util/real_point.hpp>
#include <data/font.hpp>
#include <data/draw_what.hpp>

DECLARE_POINTER_TYPE(TextElement);
DECLARE_POINTER_TYPE(Font);
class TextStyle;
class Context;
class SymbolFontRef;

// ----------------------------------------------------------------------------- : TextElement

/// Information on a linebreak
enum class LineBreak {
  NO,    // no line break ever
  MAYBE, // break here when in "direction:vertical" mode
  SPACE, // optional line break (' ')
  SOFT,  // always a line break, spacing as a soft break, doesn't end paragraphs
  HARD,  // always a line break ('\n')
  LINE,  // line break with a separator line (<line>)
};

/// Information on a character in a TextElement
struct CharInfo {
  RealSize  size;             ///< Size of this character
  LineBreak break_after : 16; ///< How/when to break after it?
  bool      soft : 1;         ///< Is this a 'soft' character? soft characters are ignored for alignment
  
  explicit CharInfo()
    : break_after(LineBreak::NO), soft(true)
  {}
  inline CharInfo(RealSize size, LineBreak break_after, bool soft = false)
    : size(size), break_after(break_after), soft(soft)
  {}
};

/// A section of text that can be rendered using a TextViewer
class TextElement : public IntrusivePtrBase<TextElement> {
public:
  /// What section of the input string is this element?
  size_t start, end;
  
  inline TextElement(size_t start ,size_t end) : start(start), end(end) {}
  virtual ~TextElement() {}
  
  /// Draw a subsection section of the text in the given rectangle
  /** xs give the x coordinates for each character
   *  this->start <= start < end <= this->end <= text.size() */
  virtual void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const = 0;
  /// Get information on all characters in the range [start...end) and store them in out
  virtual void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const = 0;
  /// Return the minimum scale factor allowed (starts at 1)
  virtual double minScale() const = 0;
  /// Return the steps the scale factor should take
  virtual double scaleStep() const = 0;
};

// ----------------------------------------------------------------------------- : SimpleTextElement

/// A text element that uses a normal font
class FontTextElement : public TextElement {
public:
  FontTextElement(const String& content, size_t start, size_t end, const FontP& font, DrawWhat draw_as, LineBreak break_style)
    : TextElement(start, end), content(content)
    , font(font), draw_as(draw_as), break_style(break_style)
  {}
  
  void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const override;
  void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const override;
  double minScale() const override;
  double scaleStep() const override;
private:
  String    content;  ///< Text to show
  FontP     font;
  DrawWhat  draw_as;
  LineBreak break_style;
};

/// A text element that uses a symbol font
class SymbolTextElement : public TextElement {
public:
  SymbolTextElement(const String& content, size_t start, size_t end, const SymbolFontRef& font, Context* ctx)
    : TextElement(start, end), content(content)
    , font(font), ctx(*ctx)
  {}
  
  void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const override;
  void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const override;
  double minScale() const override;
  double scaleStep() const override;
private:
  String content;
  const SymbolFontRef& font; // owned by TextStyle
  Context& ctx;
};

// ----------------------------------------------------------------------------- : CompoundTextElement

/// A TextElement consisting of sub elements
class CompoundTextElement : public TextElement {
public:
  CompoundTextElement(size_t start, size_t end) : TextElement(start, end) {}
  
  void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const override;
  void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const override;
  double minScale() const override;
  double scaleStep() const override;

  /// Children of this element
  /** They must be in order of positions and not overlap, i.e.
   *    i < j  ==>  elements[i].end <= elements[j].start
   */
  vector<TextElementP> children;
};

/// A TextElement drawn using a colored background
class AtomTextElement : public CompoundTextElement {
public:
  AtomTextElement(size_t start, size_t end, Color background_color) : CompoundTextElement(start, end), background_color(background_color) {}
  
  void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const override;
private:
  Color background_color;
};

/// A TextElement drawn using a red wavy underline
class ErrorTextElement : public CompoundTextElement {
public:
  ErrorTextElement(size_t start, size_t end) : CompoundTextElement(start, end) {}
  
  void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const override;
};

// ----------------------------------------------------------------------------- : TextElements

class TextParagraph {
public:
  optional<Alignment> alignment;
  double margin_left = 0., margin_right = 0.;
  //double margin_top = 0., margin_bottom = 0.; // TODO: more margin options?
  size_t start = String::npos, end = String::npos;
  size_t margin_end_char = 0; // end position of characters that are added to the margin (i.e. bullet points)
};

/// A list of text elements extracted from a string
class TextElements : public CompoundTextElement {
public:
  TextElements() : CompoundTextElement(String::npos,String::npos) {}

  /// Information on the paragraphs/blocks in the string
  /// Text segments separated by newlines are considered paragraphs
  vector<TextParagraph> paragraphs;

  void clear();
  /// Read the elements from a string
  void fromString(const String& text, const TextStyle& style, Context& ctx);
};
