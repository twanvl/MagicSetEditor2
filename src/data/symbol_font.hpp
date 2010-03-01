//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_SYMBOL_FONT
#define HEADER_DATA_SYMBOL_FONT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/alignment.hpp>
#include <util/io/package.hpp>
#include <data/font.hpp>
#include <wx/regex.h>

DECLARE_POINTER_TYPE(Font);
DECLARE_POINTER_TYPE(SymbolFont);
DECLARE_POINTER_TYPE(SymbolInFont);
DECLARE_POINTER_TYPE(InsertSymbolMenu);
class RotatedDC;
struct CharInfo;

// ----------------------------------------------------------------------------- : SymbolFont

/// A font that is drawn using images
class SymbolFont : public Packaged {
  public:
	SymbolFont();
	~SymbolFont();
	
	/// Loads the symbol font with a given name, for example "magic-mana-large"
	static SymbolFontP byName(const String& name);
	
	// Script update
	void update(Context& ctx) const;
	
	/// A symbol to be drawn
	class DrawableSymbol {
	  public:
		inline DrawableSymbol(const String& text, const String& draw_text, SymbolInFont& symbol)
			: text(text), draw_text(draw_text), symbol(&symbol)
		{}
		
		String        text;		///< Original text
		String        draw_text;///< Text to draw (extracted from the regex to avoid performance costs)
		SymbolInFont* symbol;	///< Symbol to draw
	};
	typedef vector<DrawableSymbol> SplitSymbols;
		
	/// Split a string into separate symbols for drawing and for determining their size
	void split(const String& text, SplitSymbols& out) const;
	
	/// How many consecutive characters of the text, starting at start can be rendered with this symbol font?
	size_t recognizePrefix(const String& text, size_t start) const;
	
	/// Draw a piece of text
	void draw(RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, const String& text);
	/// Get information on characters in a string
	void getCharInfo(RotatedDC& dc, Context& ctx, double font_size, const String& text, vector<CharInfo>& out);
	
	/// Draw a piece of text prepared using split
	void draw(RotatedDC& dc, RealRect rect, double font_size, const Alignment& align, const SplitSymbols& text);
	/// Get information on characters in a string
	void getCharInfo(RotatedDC& dc, double font_size, const SplitSymbols& text, vector<CharInfo>& out);
	
	/// Get the image for a symbol
	Image getImage(double font_size, const DrawableSymbol& symbol);
	
	static String typeNameStatic();
	virtual String typeName() const;
	Version fileVersion() const;
	
	/// Generate a 'insert symbol' menu.
	/** This class owns the menu!
	 *  All ids used will be in the range ID_INSERT_SYMBOL_MENU_MIN...ID_INSERT_SYMBOL_MENU_MAX.
	 *  If there is no insert symbol menu, returns nullptr.
	 */
	wxMenu* insertSymbolMenu(Context& ctx);
	/// Process a choice from the insert symbol menu
	/** Return the code representing the symbol */
	String insertSymbolCode(int menu_id) const;
		
  private:
	double img_size;	///< Font size that the images use
	RealSize spacing;	///< Spacing between sybmols (for the default font size)
	// writing text
	bool scale_text;	///< Should text be scaled down to fit in a symbol?
	InsertSymbolMenuP insert_symbol_menu;
	wxMenu* processed_insert_symbol_menu;
	
	friend class SymbolInFont;
	friend class InsertSymbolMenu;
	vector<SymbolInFontP> symbols;	///< The individual symbols
		
	/// Find the default symbol
	/** may return nullptr */
	SymbolInFont* defaultSymbol() const;
	
	/// Draws a single symbol inside the given rectangle
	void drawSymbol  (RotatedDC& dc, RealRect sym_rect, double font_size, const Alignment& align, SymbolInFont& sym, const String& text);
	
	/// Size of a single symbol, including spacing
	RealSize symbolSize       (double font_size, const DrawableSymbol& sym);
  public:
	/// The default size of symbols, including spacing
	RealSize defaultSymbolSize(double font_size);
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : InsertSymbolMenu

enum MenuItemType
{	ITEM_CODE		///< Name gives the code to insert
,	ITEM_CUSTOM		///< Use a dialog box
,	ITEM_LINE		///< A menu separator
,	ITEM_SUBMENU	///< A submenu
};

/// Description of a menu to insert symbols from a symbol font into the text
class InsertSymbolMenu : public IntrusivePtrBase<InsertSymbolMenu> {
  public:
	InsertSymbolMenu();
	
	MenuItemType              type;
	String                    name;
	vector<InsertSymbolMenuP> items;
	
	/// Number of ids used (recursive)
	int size() const;
	/// Get the code for an item, id relative to the start of this menu
	String getCode(int id, const SymbolFont& font) const;
	/// Make an actual menu
	wxMenu*     makeMenu(int first_id, SymbolFont& font) const;
	/// Make an actual menu item
	wxMenuItem* makeMenuItem(wxMenu* parent, int first_id, SymbolFont& font) const;
	
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
		
	Scriptable<String>    name;				///< Font package name, can be changed with script
	Scriptable<double>    size;				///< Size of the font
	double                scale_down_to;	///< Mimumum size of the font
	Scriptable<Alignment> alignment;		///< Alignment of symbols in a line of text
	SymbolFontP           font;				///< The font, if it is loaded
	
  private:
	DECLARE_REFLECTION();
	
	/// (re)load the symbol font based on name
	void loadFont(Context& ctx);
};

// ----------------------------------------------------------------------------- : EOF
#endif
