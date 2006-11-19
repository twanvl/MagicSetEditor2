//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/symbol_font.hpp>
#include <util/dynamic_arg.hpp>
#include <util/io/package_manager.hpp>
#include <script/image.hpp>

// ----------------------------------------------------------------------------- : SymbolFont

// SymbolFont that is used for SymbolInFonts constructed with the default constructor
DECLARE_DYNAMIC_ARG(SymbolFont*, symbol_font_for_reading);
IMPLEMENT_DYNAMIC_ARG(SymbolFont*, symbol_font_for_reading, nullptr);

SymbolFont::SymbolFont()
	: img_size(12), min_size(1)
	, spacing(1,1)
	, scale_text(false)
	, text_margin_left(0), text_margin_right(0)
	, text_margin_top(0),  text_margin_bottom(0)
	, text_alignment(ALIGN_MIDDLE_CENTER)
	, merge_numbers(false)
{}

String SymbolFont::typeNameStatic() { return _("symbol-font"); }
String SymbolFont::typeName() const { return _("symbol-font"); }

SymbolFontP SymbolFont::byName(const String& name) {
	return packages.open<SymbolFont>(name + _(".mse-symbol-font"));
}

void SymbolFont::split(const String& text, SplitSymbols& out) {
	for (size_t pos = 0 ; pos < text.size() ; ) {
		// read a single symbol
		pos = pos + 1; // TODO
	}
}

IMPLEMENT_REFLECTION(SymbolFont) {
	tag.addAlias(300, _("text align"), _("text alignment"));
	
	REFLECT_N("image font size",  img_size);
	REFLECT_N("scale down to",    min_size);
	REFLECT_N("horizontal space", spacing.width);
	REFLECT_N("vertical space",   spacing.height);
	WITH_DYNAMIC_ARG(symbol_font_for_reading, this);
	  REFLECT(symbols);
	REFLECT(text_font);
	REFLECT(scale_text);
	REFLECT(merge_numbers);
	REFLECT(text_margin_left);
	REFLECT(text_margin_right);
	REFLECT(text_margin_top);
	REFLECT(text_margin_bottom);
	REFLECT(text_alignment);
}

// ----------------------------------------------------------------------------- : SymbolFont::Symbol

/// A symbol in a symbol font
class SymbolInFont {
  public:
	SymbolInFont();
	
	/// Get a shrunk, zoomed bitmap
	Bitmap getBitmap(Context& ctx, double size);
	
	/// Size of a (zoomed) bitmap
	/** This is the size of the resulting image, it does NOT convert back to internal coordinates */
	RealSize size(Context& ctx, double size);
	
  private:
	String          code;			///< Code for this symbol
	ScriptableImage image;			///< The image for this symbol
	UInt            img_size;		///< Font size used by the image
	wxSize          actual_size;	///< Actual image size, only known after loading the image
	/// Cached bitmaps for different sizes
	map<double, Bitmap> bitmaps;
	
	DECLARE_REFLECTION();
};

SymbolInFont::SymbolInFont()
{
	assert(symbol_font_for_reading());
	img_size = symbol_font_for_reading()->img_size;
}

IMPLEMENT_REFLECTION(SymbolInFont) {
	REFLECT(code);
	REFLECT(image);
	REFLECT_N("image font size", img_size);
}

// ----------------------------------------------------------------------------- : SymbolFontRef

SymbolFontRef::SymbolFontRef()
	: size(12)
	, scale_down_to(1)
	, alignment(ALIGN_MIDDLE_CENTER)
{}

bool SymbolFontRef::valid() const {
	return !!font; //TODO: does this make sense?
}


IMPLEMENT_REFLECTION(SymbolFontRef) {
	REFLECT(name);
	REFLECT(size);
	REFLECT(scale_down_to);
	REFLECT(alignment);
}
