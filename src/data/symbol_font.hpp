//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_SYMBOL_FONT
#define HEADER_DATA_SYMBOL_FONT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/alignment.hpp>
#include <util/io/package.hpp>
#include <data/font.hpp>

DECLARE_POINTER_TYPE(Font);
DECLARE_POINTER_TYPE(SymbolFont);
DECLARE_POINTER_TYPE(SymbolInFont);
class RotatedDC;
struct CharInfo;

// ----------------------------------------------------------------------------- : SymbolFont

// A font that is drawn using images
class SymbolFont : public Packaged {
  public:
	SymbolFont();
	
	/// Loads the symbol font with a given name, for example "magic-mana-large"
	static SymbolFontP byName(const String& name);
	
	class DrawableSymbol;
	typedef vector<DrawableSymbol> SplitSymbols;
	/// Split a string into separate symbols for drawing and for determining their size
	void split(const String& text, SplitSymbols& out) const;
	
	/// Draw a piece of text prepared using split
	void draw(RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, const String& text);
	/// Get information on characters in a string
	void getCharInfo(RotatedDC& dc, Context& ctx, double font_size, const String& text, vector<CharInfo>& out);
	
	/// Draw a piece of text prepared using split
	void draw(RotatedDC& dc, Context& ctx, RealRect rect, double font_size, const Alignment& align, const SplitSymbols& text);
	/// Get information on characters in a string
	void getCharInfo(RotatedDC& dc, Context& ctx, double font_size, const SplitSymbols& text, vector<CharInfo>& out);
	
	static String typeNameStatic();
	virtual String typeName() const;
	
  private:
	UInt img_size;		///< Font size that the images use
	UInt min_size;		///< Minimum font size
	RealSize spacing;	///< Spacing between sybmols (for the default font size)
	// writing text
	bool scale_text;	///< Should text be scaled down to fit in a symbol?
	FontP text_font;	///< Font to use for missing symbols
	double text_margin_left;
	double text_margin_right;
	double text_margin_top;
	double text_margin_bottom;
	Alignment text_alignment;
	bool merge_numbers;	///< Merge numbers? e.g. "11" is a single symbol ('1' must not exist as a symbol)
	
	friend class SymbolInFont;
	vector<SymbolInFontP> symbols;	///< The individual symbols
	
	/// Find the default symbol
	/** may return nullptr */
	SymbolInFont* defaultSymbol() const;
	
	/// Draws a single symbol inside the given rectangle
	void drawSymbol  (RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, SymbolInFont& sym);
	/// Draw the default bitmap to a dc and overlay a string of text
	void drawWithText(RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, const String& text);
	
	/// Size of a single symbol
	RealSize symbolSize       (Context& ctx, double font_size, const DrawableSymbol& sym);
  public:
	/// Size of the default symbol
	RealSize defaultSymbolSize(Context& ctx, double font_size);
	
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : SymbolFontRef

/// A reference to an actual symbol font
class SymbolFontRef {
  public:
	SymbolFontRef();
	
	// Script update
	bool update(Context& ctx);
	void initDependencies(Context&, const Dependency&) const;
	
	/// Is a font loaded?
	bool valid() const;
		
	Scriptable<String> name;			///< Font package name, can be changed with script
	double             size;			///< Size of the font
	double             scale_down_to;	///< Mimumum size of the font
	Alignment          alignment;		///< Alignment of symbols in a line of text
	SymbolFontP        font;			///< The font, if it is loaded
	
	/// (re)load the symbol font based on name
	void loadFont();
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
