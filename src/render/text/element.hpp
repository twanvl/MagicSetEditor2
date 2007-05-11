//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_TEXT_ELEMENT
#define HEADER_RENDER_TEXT_ELEMENT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <util/real_point.hpp>
#include <data/font.hpp>

DECLARE_POINTER_TYPE(TextElement);
DECLARE_POINTER_TYPE(Font);
class TextStyle;
class Context;
class SymbolFontRef;

// ----------------------------------------------------------------------------- : TextElement

/// What should be drawn?
enum DrawWhat
{	DRAW_NOTHING = 0x00
,	DRAW_NORMAL  = 0x01 // draw normal things, like the text
,	DRAW_BORDERS = 0x02 // draw editor stuff, such as borders/lines
,	DRAW_ACTIVE  = 0x04 // draw active editor stuff, such as hidden separators and atom highlights
};

/// Information on a linebreak
enum LineBreak
{	BREAK_NO		// no line break ever
,	BREAK_MAYBE		// break here when in "direction:vertical" mode
,	BREAK_SOFT		// optional line break (' ')
,	BREAK_HARD		// always a line break ('\n')
,	BREAK_LINE		// line break with a separator line (<line>)
};

/// Information on a character in a TextElement
struct CharInfo {
	RealSize  size;
	LineBreak break_after;
	
	inline CharInfo(RealSize size, LineBreak break_after) : size(size), break_after(break_after) {}
};

/// A section of text that can be rendered using a TextViewer
class TextElement : public IntrusivePtrBase<TextElement> {
  public:
	/// The text of which a subsection is drawn
	String text;
	/// What section of the input string is this element?
	size_t start, end;
	
	inline TextElement(const String& text, size_t start ,size_t end) : text(text), start(start), end(end) {}
	virtual ~TextElement() {}
	
	/// Draw a subsection section of the text in the given rectangle
	/** xs give the x coordinates for each character
	 *  this->start <= start < end <= this->end <= text.size() */
	virtual void draw       (RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const = 0;
	/// Get information on all characters in the range [start...end) and store them in out
	virtual void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const = 0;
	/// Return the minimum scale factor allowed (starts at 1)
	virtual double minScale() const = 0;
	/// Return the steps the scale factor should take
	virtual double scaleStep() const = 0;
};

// ----------------------------------------------------------------------------- : TextElements

/// A list of text elements
class TextElements : public vector<TextElementP> {
  public:
	/// Draw all the elements (as need to show the range start..end)
	void draw       (RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
	// Get information on all characters in the range [start...end) and store them in out
	void getCharInfo(RotatedDC& dc, double scale, size_t start, size_t end, vector<CharInfo>& out) const;
	/// Return the minimum scale factor allowed by all elements
	double minScale() const;
	/// Return the steps the scale factor should take
	double scaleStep() const;
	
	/// The actual elements
	/** They must be in order of positions and not overlap, i.e.
	 *    i < j  ==>  elements[i].end <= elements[j].start
	 */
	vector<TextElementP> elements;
	
	/// Find the element that contains the given index, if there is any
	vector<TextElementP>::const_iterator findByIndex(size_t index) const;
	
	/// Read the elements from a string
	void fromString(const String& text, size_t start, size_t end, const TextStyle& style, Context& ctx);
};

// ----------------------------------------------------------------------------- : SimpleTextElement

/// A text element that just shows text
class SimpleTextElement : public TextElement {
  public:
	SimpleTextElement(const String& text, size_t start, size_t end)
		: TextElement(text, start, end), content(text.substr(start,end-start))
	{}
	String content;	///< Text to show
};

/// A text element that uses a normal font
class FontTextElement : public SimpleTextElement {
  public:
	FontTextElement(const String& text, size_t start ,size_t end, const FontP& font, DrawWhat draw_as, LineBreak break_style)
		: SimpleTextElement(text, start, end)
		, font(font), draw_as(draw_as), break_style(break_style)
	{}
	
	virtual void draw       (RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
	virtual void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const;
	virtual double minScale() const;
	virtual double scaleStep() const;
  private:
	FontP     font;
	DrawWhat  draw_as;
	LineBreak break_style;
};

/// A text element that uses a symbol font
class SymbolTextElement : public SimpleTextElement {
  public:
	SymbolTextElement(const String& text, size_t start ,size_t end, const SymbolFontRef& font, Context* ctx)
		: SimpleTextElement(text, start, end)
		, font(font), ctx(*ctx)
	{}
	
	virtual void draw       (RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
	virtual void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const;
	virtual double minScale() const;
	virtual double scaleStep() const;
  private:
	const SymbolFontRef& font; // owned by TextStyle
	Context& ctx;
};

// ----------------------------------------------------------------------------- : CompoundTextElement

/// A TextElement consisting of sub elements
class CompoundTextElement : public TextElement {
  public:
	CompoundTextElement(const String& text, size_t start ,size_t end) : TextElement(text, start, end) {}
	
	virtual void draw       (RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
	virtual void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const;
	virtual double minScale() const;
	virtual double scaleStep() const;
	
	TextElements elements; ///< the elements
};

/// A TextElement drawn using a grey background
class AtomTextElement : public CompoundTextElement {
  public:
	AtomTextElement(const String& text, size_t start ,size_t end) : CompoundTextElement(text, start, end) {}
	
	virtual void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
};

/// A TextElement drawn using a red wavy underline
class ErrorTextElement : public CompoundTextElement {
  public:
	ErrorTextElement(const String& text, size_t start ,size_t end) : CompoundTextElement(text, start, end) {}
	
	virtual void draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
};

// ----------------------------------------------------------------------------- : Other text elements
/*
/// A text element that displays a horizontal separator line
class HorizontalLineTextElement : public TextElement {
  public:
	HorizontalLineTextElement(const String& text, size_t start ,size_t end) : TextElement(text, start, end) {}

	virtual void draw       (RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const;
	virtual void getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const;
	virtual double minScale() const;
};
*/

// ----------------------------------------------------------------------------- : EOF
#endif
