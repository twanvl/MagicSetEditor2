//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/symbol_font.hpp>
#include <util/dynamic_arg.hpp>
#include <util/io/package_manager.hpp>
#include <util/rotation.hpp>
#include <render/text/element.hpp> // fot CharInfo
#include <script/image.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolFont::DrawableSymbol);
DECLARE_TYPEOF_COLLECTION(SymbolInFontP);

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

// ----------------------------------------------------------------------------- : SymbolInFont

/// A symbol in a symbol font
class SymbolInFont {
  public:
	SymbolInFont();
	
	/// Get a shrunk, zoomed bitmap
	Bitmap getBitmap(Context& ctx, Package& pkg, double size);
	
	/// Size of a (zoomed) bitmap
	/** This is the size of the resulting image, it does NOT convert back to internal coordinates */
	RealSize size(Context& ctx, Package& pkg, double size);
	
	String          code;			///< Code for this symbol
  private:
	ScriptableImage image;			///< The image for this symbol
	double          img_size;		///< Font size used by the image
	wxSize          actual_size;	///< Actual image size, only known after loading the image
	/// Cached bitmaps for different sizes
	map<double, Bitmap> bitmaps;
	
	DECLARE_REFLECTION();
};

SymbolInFont::SymbolInFont()
	: actual_size(0,0)
{
	assert(symbol_font_for_reading());
	img_size = symbol_font_for_reading()->img_size;
	if (img_size <= 0) img_size = 1;
}

Bitmap SymbolInFont::getBitmap(Context& ctx, Package& pkg, double size) {
	// is this bitmap already loaded/generated?
	Bitmap& bmp = bitmaps[size];
	if (!bmp.Ok()) {
		// generate new bitmap
		if (!image) {
			throw Error(_("No image specified for symbol with code '") + code + _("' in symbol font."));
		}
		Image img = image.generate(ctx, pkg)->image;
		actual_size = wxSize(img.GetWidth(), img.GetHeight());
		// scale to match expected size
		Image resampled_image(actual_size.GetWidth()  * size / img_size,
			                  actual_size.GetHeight() * size / img_size, false);
		if (!resampled_image.Ok()) return Bitmap(1,1);
		resample(img, resampled_image);
		// convert to bitmap, store for later use
		bmp = Bitmap(resampled_image);
	}
	return bmp;
}

RealSize SymbolInFont::size(Context& ctx, Package& pkg, double size) {
	if (actual_size.GetWidth() == 0) {
		// we don't know what size the image will be
		getBitmap(ctx, pkg, size);
	}
	return wxSize(actual_size * size / img_size);
}

IMPLEMENT_REFLECTION(SymbolInFont) {
	REFLECT(code);
	REFLECT(image);
	REFLECT_N("image font size", img_size);
}

// ----------------------------------------------------------------------------- : SymbolFont : splitting

class SymbolFont::DrawableSymbol {
  public:
	DrawableSymbol(const String& text, SymbolInFont* symbol)
		: text(text), symbol(symbol)
	{}
	
	String        text;		///< Original text
	SymbolInFont* symbol;	///< Symbol to draw, if nullptr, use the default symbol and draw the text
};

void SymbolFont::split(const String& text, SplitSymbols& out) const {
	// read a single symbol until we are done with the text
	for (size_t pos = 0 ; pos < text.size() ; ) {
		// 1. check merged numbers
		if (merge_numbers && pos + 1 < text.size()) {
			size_t num_count = text.find_first_not_of(_("0123456789"), pos) - pos;
			if (num_count >= 2) {
				// draw single symbol for the whole number
				out.push_back(DrawableSymbol(text.substr(pos, num_count), nullptr));
				pos += num_count;
				goto next_symbol;
			}
		}
		// 2. check symbol list
		FOR_EACH_CONST(sym, symbols) {
			if (!sym->code.empty() && is_substr(text, pos, sym->code)) { // symbol matches
				out.push_back(DrawableSymbol(sym->code, sym.get()));
				pos += sym->code.size();
				goto next_symbol; // continue two levels
			}
		}
		// 3. unknown code, draw single character as text
		out.push_back(DrawableSymbol(text.substr(pos, 1), 0));
		pos += 1;
next_symbol:;
	}
}

SymbolInFont* SymbolFont::defaultSymbol() const {
	FOR_EACH_CONST(sym, symbols) {
		if (sym->code.empty()) return sym.get();
	}
	return nullptr;
}

// ----------------------------------------------------------------------------- : SymbolFont : drawing

void SymbolFont::draw(RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, const String& text) {
	SplitSymbols symbols;
	split(text, symbols);
	draw(dc, ctx, rect, font_size, align, symbols);
}

void SymbolFont::draw(RotatedDC& dc, Context& ctx, RealRect rect, double font_size, const Alignment& align, const SplitSymbols& text) {
	FOR_EACH_CONST(sym, text) {
		RealSize size = dc.trInvS(symbolSize(ctx, dc.trS(font_size), sym));
		RealRect sym_rect = split_left(rect, size);
		if (sym.symbol) {
			drawSymbol(  dc, ctx, sym_rect, font_size, align, *sym.symbol);
		} else {
			drawWithText(dc, ctx, sym_rect, font_size, align, sym.text);
		}
	}
}

void SymbolFont::drawSymbol  (RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, SymbolInFont& sym) {
	// find bitmap
	Bitmap bmp = sym.getBitmap(ctx, *this, dc.trS(font_size));
	// draw aligned in the rectangle
	dc.DrawBitmap(bmp, align_in_rect(align, dc.trInvS(RealSize(bmp.GetWidth(), bmp.GetHeight())), rect));
}

void SymbolFont::drawWithText(RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, const String& text) {
	// 1. draw background bitmap
	// Size and position of symbol
	RealRect sym_rect = rect;
	// find and draw background bitmap
	SymbolInFont* def = defaultSymbol();
	if (def) {
		Bitmap bmp = def->getBitmap(ctx, *this, dc.trS(font_size));
		// align symbol
		sym_rect.size()     = dc.trInvS(RealSize(bmp.GetWidth(), bmp.GetHeight()));
		sym_rect.position() = align_in_rect(align, sym_rect.size(), rect);
		// draw
		dc.DrawBitmap(bmp, sym_rect.position());
	}
	
	// 2. draw text
	if (!text_font) return;
	// subtract margins from size
	sym_rect.x      += text_margin_left;
	sym_rect.y      += text_margin_top;
	sym_rect.width  -= text_margin_left + text_margin_right;
	sym_rect.height -= text_margin_top  + text_margin_bottom;
	// setup text, shrink it
	double size = text_font->size; // TODO : incorporate shrink factor?
	RealSize ts;
	while (true) {
		if (size <= 0) return; // text too small
		dc.SetFont(text_font->font, size);
		ts = dc.GetTextExtent(text);
		if (ts.width <= sym_rect.width && ts.height <= sym_rect.height) {
			break; // text fits
		} else {
			// text doesn't fit
			size -= dc.getFontSizeStep();
		}
	}
	// align text
	RealPoint text_pos = align_in_rect(text_alignment, ts, sym_rect);
	// draw text
	if (text_font->hasShadow()) {
		dc.SetTextForeground(text_font->shadow_color);
		dc.DrawText(text, text_pos + text_font->shadow_displacement);
	}
	dc.SetTextForeground(text_font->color);
	dc.DrawText(text, text_pos);
}


// ----------------------------------------------------------------------------- : SymbolFont : sizes

void SymbolFont::getCharInfo(RotatedDC& dc, Context& ctx, double font_size, const String& text, vector<CharInfo>& out) {
	SplitSymbols symbols;
	split(text, symbols);
	getCharInfo(dc, ctx, font_size, symbols, out);
}

void SymbolFont::getCharInfo(RotatedDC& dc, Context& ctx, double font_size, const SplitSymbols& text, vector<CharInfo>& out) {
	FOR_EACH_CONST(sym, text) {
		size_t count = sym.text.size();
		RealSize size = dc.trInvS(symbolSize(ctx, dc.trS(font_size), sym));
		out.insert(out.end(), count, RealSize(size.width / count, size.height)); // divide into count parts
	}
}

RealSize SymbolFont::symbolSize(Context& ctx, double font_size, const DrawableSymbol& sym) {
	if (sym.symbol) {
		return addDiagonal(sym.symbol->size(ctx, *this, font_size), spacing);
	} else {
		return defaultSymbolSize(ctx, font_size);
	}
}

RealSize SymbolFont::defaultSymbolSize(Context& ctx, double font_size) {
	SymbolInFont* def = defaultSymbol();
	if (def) {
		return addDiagonal(def->size(ctx, *this, font_size), spacing);
	} else {
		return addDiagonal(RealSize(1,1), spacing);
	}
}


// ----------------------------------------------------------------------------- : SymbolFontRef

SymbolFontRef::SymbolFontRef()
	: size(12)
	, scale_down_to(1)
	, alignment(ALIGN_MIDDLE_CENTER)
{}

bool SymbolFontRef::valid() const {
	return !!font;
}

bool SymbolFontRef::update(Context& ctx) {
	if (name.update(ctx)) {
		// font name changed, load another font
		loadFont();
		return true;
	} else {
		if (!font) loadFont();
		return false;
	}
}
void SymbolFontRef::initDependencies(Context& ctx, const Dependency& dep) const {
	name.initDependencies(ctx, dep);
}

void SymbolFontRef::loadFont() {
	if (name().empty()) {
		font = SymbolFontP();
	} else {
		font = SymbolFont::byName(name);
	}
}

IMPLEMENT_REFLECTION(SymbolFontRef) {
	REFLECT(name);
	REFLECT(size);
	REFLECT(scale_down_to);
	REFLECT(alignment);
}
