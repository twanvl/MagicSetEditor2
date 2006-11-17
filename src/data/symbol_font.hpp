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

// ----------------------------------------------------------------------------- : SymbolFont

// A font that is drawn using images
class SymbolFont : Packaged {
  public:
	/// Loads the symbol font with a given name, for example "magic-mana-large"
	static SymbolFontP byName(const String& name);
	
  private:
	UInt imgSize;		///< Font size that the images use
	UInt minSize;		///< Minimum font size
	RealSize spacing;	///< Spacing between sybmols (for the default font size)
	// writing text
	bool scale_text;	///< Should text be scaled down to fit in a symbol?
	FontP text_font;	///< Font to use for missing symbols
	double text_margin_left;
	double text_margin_right;
	double text_margin_rop;
	double text_margin_bottom;
	Alignment text_align;
	bool merge_numbers;	///< Merge numbers? e.g. "11" is a single symbol ('1' must not exist as a symbol)
	
  public:
	class Symbol;
  private:
	vector<SymbolInFontP> symbols;	///< The individual symbols
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : SymbolFontRef

/// A reference to an actual symbol font
class SymbolFontRef {
  public:
	SymbolFontRef();
	
	// Script update
	bool update(Context& ctx);
	void initDependencies(Context&, Dependency& dep);
	
	/// Is a font loaded?
	bool valid();
	
  private:
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
