//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol_font.hpp>
#include <data/stylesheet.hpp>
#include <util/dynamic_arg.hpp>
#include <util/io/package_manager.hpp>
#include <util/rotation.hpp>
#include <util/error.hpp>
#include <util/window_id.hpp>
#include <render/text/element.hpp> // fot CharInfo
#include <script/image.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolFont::DrawableSymbol);
DECLARE_TYPEOF_COLLECTION(SymbolInFontP);
DECLARE_TYPEOF_COLLECTION(InsertSymbolMenuP);

// ----------------------------------------------------------------------------- : SymbolFont

// SymbolFont that is used for SymbolInFonts constructed with the default constructor
DECLARE_DYNAMIC_ARG(SymbolFont*, symbol_font_for_reading);
IMPLEMENT_DYNAMIC_ARG(SymbolFont*, symbol_font_for_reading, nullptr);

SymbolFont::SymbolFont()
	: img_size(12)
	, spacing(1,1)
	, scale_text(false)
	, processed_insert_symbol_menu(nullptr)
{}

SymbolFont::~SymbolFont() {
	delete processed_insert_symbol_menu;
}

String SymbolFont::typeNameStatic() { return _("symbol-font"); }
String SymbolFont::typeName() const { return _("symbol-font"); }
Version SymbolFont::fileVersion() const { return file_version_symbol_font; }

SymbolFontP SymbolFont::byName(const String& name) {
	return package_manager.open<SymbolFont>(
		name.size() > 16 && is_substr(name, name.size() - 16, _(".mse-symbol-font"))
		? name : name + _(".mse-symbol-font"));
}

IMPLEMENT_REFLECTION(SymbolFont) {
	REFLECT_BASE(Packaged);

	REFLECT_ALIAS(300, "text align", "text alignment");
	
	REFLECT_N("image font size",  img_size);
	REFLECT_N("horizontal space", spacing.width);
	REFLECT_N("vertical space",   spacing.height);
	WITH_DYNAMIC_ARG(symbol_font_for_reading, this);
	  REFLECT(symbols);
	REFLECT(scale_text);
	REFLECT(insert_symbol_menu);
}

// ----------------------------------------------------------------------------- : SymbolInFont

/// A symbol in a symbol font
class SymbolInFont : public IntrusivePtrBase<SymbolInFont> {
  public:
	SymbolInFont();
	
	/// Get a shrunk, zoomed image
	Image getImage(Package& pkg, double size);
	
	/// Get a shrunk, zoomed bitmap
	Bitmap getBitmap(Package& pkg, double size);
	
	/// Get a bitmap with the given size
	Bitmap getBitmap(Package& pkg, wxSize size);
	
	/// Size of a (zoomed) bitmap
	/** This is the size of the resulting image, it does NOT convert back to internal coordinates */
	RealSize size(Package& pkg, double size);
	
	void update(Context& ctx);
	
	String           code;			///< Code for this symbol
	Scriptable<bool> enabled;		///< Is this symbol enabled?
	bool             regex;			///< Should this symbol be matched by a regex?
	int              draw_text;		///< The index of the captured regex expression to draw, or -1 to not draw text
	wxRegEx          code_regex;	///< Regex for matching the symbol code
	FontP            text_font;		///< Font to draw text in.
	Alignment        text_alignment;
	double           text_margin_left;
	double           text_margin_right;
	double           text_margin_top;
	double           text_margin_bottom;
  private:
	ScriptableImage  image;			///< The image for this symbol
	double           img_size;		///< Font size used by the image
	wxSize           actual_size;	///< Actual image size, only known after loading the image
	/// Cached bitmaps for different sizes
	map<double, Bitmap> bitmaps;
	
	DECLARE_REFLECTION();
};

SymbolInFont::SymbolInFont()
	: enabled(true)
	, regex(false)
	, draw_text(-1)
	, code_regex()
	, text_alignment(ALIGN_MIDDLE_CENTER)
	, text_margin_left(0), text_margin_right(0)
	, text_margin_top(0),  text_margin_bottom(0)
	, actual_size(0,0)
{
	assert(symbol_font_for_reading());
	img_size = symbol_font_for_reading()->img_size;
	if (img_size <= 0) img_size = 1;
}

Image SymbolInFont::getImage(Package& pkg, double size) {
	// generate new image
	if (!image.isReady()) {
		throw Error(_("No image specified for symbol with code '") + code + _("' in symbol font."));
	}
	Image img = image.generate(GeneratedImage::Options(0, 0, &pkg));
	actual_size = wxSize(img.GetWidth(), img.GetHeight());
	// scale to match expected size
	Image resampled_image((int) (actual_size.GetWidth()  * size / img_size),
	                      (int) (actual_size.GetHeight() * size / img_size), false);
	if (!resampled_image.Ok()) return Image(1,1);
	resample(img, resampled_image);
	return resampled_image;
}
Bitmap SymbolInFont::getBitmap(Package& pkg, double size) {
	// is this bitmap already loaded/generated?
	Bitmap& bmp = bitmaps[size];
	if (!bmp.Ok()) {
		// generate image, convert to bitmap, store for later use
		bmp = Bitmap(getImage(pkg, size));
	}
	return bmp;
}
Bitmap SymbolInFont::getBitmap(Package& pkg, wxSize size) {
	// generate new bitmap
	if (!image.isReady()) {
		throw Error(_("No image specified for symbol with code '") + code + _("' in symbol font."));
	}
	return Bitmap( image.generate(GeneratedImage::Options(size.x, size.y, &pkg, nullptr, ASPECT_BORDER)) );
}

RealSize SymbolInFont::size(Package& pkg, double size) {
	if (actual_size.GetWidth() == 0) {
		// we don't know what size the image will be
		getBitmap(pkg, size);
	}
	return wxSize(actual_size * (int) (size) / (int) (img_size));
}

void SymbolInFont::update(Context& ctx) {
	image.update(ctx);
	enabled.update(ctx);
	if (text_font)
		text_font->update(ctx);
}
void SymbolFont::update(Context& ctx) const {
	// update all symbol-in-fonts
	FOR_EACH_CONST(sym, symbols) {
		sym->update(ctx);
	}
}

IMPLEMENT_REFLECTION(SymbolInFont) {
	REFLECT(code);
	REFLECT(regex);
	REFLECT_IF_READING
		if (regex)
			code_regex.Compile(code, wxRE_ADVANCED);
	REFLECT(draw_text);
	REFLECT(text_font);
	REFLECT(text_alignment);
	REFLECT(text_margin_left);
	REFLECT(text_margin_right);
	REFLECT(text_margin_top);
	REFLECT(text_margin_bottom);
	REFLECT(image);
	REFLECT(enabled);
	REFLECT_N("image font size", img_size);
}

// ----------------------------------------------------------------------------- : SymbolFont : splitting

void SymbolFont::split(const String& text, SplitSymbols& out) const {
	// read a single symbol until we are done with the text
	for (size_t pos = 0 ; pos < text.size() ; ) {
		// check symbol list
		FOR_EACH_CONST(sym, symbols) {
			if (!sym->code.empty() && sym->enabled) {
				size_t start, len = 1;
				if (sym->regex && sym->code_regex.IsValid() && sym->code_regex.Matches(text.substr(pos), wxRE_NOTBOL | wxRE_NOTEOL) &&
						sym->code_regex.GetMatch(&start, &len) && start == 0 && len > 0) { //Matches the regex
					if (sym->draw_text >= 0 && sym->draw_text < (int)sym->code_regex.GetMatchCount()) {
						size_t draw_end;
						sym->code_regex.GetMatch(&start, &draw_end, sym->draw_text);
						out.push_back(DrawableSymbol(
										text.substr(pos, len),
										text.substr(pos + start, draw_end - start),
										*sym));
					} else {
						out.push_back(DrawableSymbol(
										text.substr(pos, len),
										_(""),
										*sym));
					}
					pos += len;
					goto next_symbol;
				} else if (is_substr(text, pos, sym->code)) {
					out.push_back(DrawableSymbol(sym->code, sym->draw_text >= 0 ? sym->code : _(""), *sym));
					pos += sym->code.size();
					goto next_symbol; // continue two levels
				}
			}
		}
		// unknown code, draw single character as text
		//out.push_back(DrawableSymbol(text.substr(pos, 1), _(""), defaultSymbol()));
		pos += 1;
next_symbol:;
	}
}

size_t SymbolFont::recognizePrefix(const String& text, size_t start) const {
	size_t pos;
	for (pos = start ; pos < text.size() ; ) {
		// check symbol list
		FOR_EACH_CONST(sym, symbols) {
			if (!sym->code.empty() && sym->enabled) {
				size_t start, len = 1;
				if (sym->regex && sym->code_regex.IsValid() && sym->code_regex.Matches(text.substr(pos), wxRE_NOTBOL | wxRE_NOTEOL) &&
						sym->code_regex.GetMatch(&start, &len) && start == 0 && len > 0) { //Matches the regex
					pos += len;
					goto next_symbol;
				} else if (is_substr(text, pos, sym->code)) {
					pos += sym->code.size();
					goto next_symbol; // continue two levels
				}
			}
		}
		// failed
		break;
next_symbol:;
	}
	return pos - start;
}

SymbolInFont* SymbolFont::defaultSymbol() const {
	FOR_EACH_CONST(sym, symbols) {
		if (sym->regex && sym->code_regex.Matches(_("0")) && sym->enabled) return sym.get();
	}
	return nullptr;
}

// ----------------------------------------------------------------------------- : SymbolFont : drawing

void SymbolFont::draw(RotatedDC& dc, Context& ctx, const RealRect& rect, double font_size, const Alignment& align, const String& text) {
	SplitSymbols symbols;
	update(ctx);
	split(text, symbols);
	draw(dc, rect, font_size, align, symbols);
}

void SymbolFont::draw(RotatedDC& dc, RealRect rect, double font_size, const Alignment& align, const SplitSymbols& text) {
	FOR_EACH_CONST(sym, text) {
		RealSize size = dc.trInvS(symbolSize(dc.trS(font_size), sym));
		RealRect sym_rect = split_left(rect, size);
		drawSymbol(dc, sym_rect, font_size, align, *sym.symbol, sym.draw_text);
	}
}

void SymbolFont::drawSymbol(RotatedDC& dc, RealRect sym_rect, double font_size, const Alignment& align, SymbolInFont& sym, const String& text) {
	// 1. draw symbol
	// find bitmap
	Bitmap bmp = sym.getBitmap(*this, dc.trS(font_size));
	// draw aligned in the rectangle
	RealSize  bmp_size = dc.trInvS(RealSize(bmp));
	RealPoint bmp_pos  = align_in_rect(align, bmp_size, sym_rect);
	dc.DrawBitmap(bmp, bmp_pos);
	
	// 2. draw text
	if (text.empty() || !sym.text_font) return;
	// only use the bitmap rectangle
	sym_rect = RealRect(bmp_pos, bmp_size);
	// subtract margins from size
	sym_rect.x      += font_size * sym.text_margin_left;
	sym_rect.y      += font_size * sym.text_margin_top;
	sym_rect.width  -= font_size * (sym.text_margin_left + sym.text_margin_right);
	sym_rect.height -= font_size * (sym.text_margin_top  + sym.text_margin_bottom);
	// setup text, shrink it
	double size = font_size * sym.text_font->size;
	double stretch = 1.0;
	RealSize ts;
	while (true) {
		if (size <= 0) return; // text too small
		dc.SetFont(*sym.text_font, size / sym.text_font->size);
		ts = dc.GetTextExtent(text);
		if (ts.height <= sym_rect.height) {
			if (ts.width <= sym_rect.width) {
				break; // text fits
			} else if (ts.width * sym.text_font->max_stretch <= sym_rect.width) {
				stretch = sym_rect.width / ts.width;
				ts.width = sym_rect.width; // for alignment
				break;
			}
		}
		// text doesn't fit
		size -= dc.getFontSizeStep();
	}
	// align text
	RealPoint text_pos = align_in_rect(sym.text_alignment, ts, sym_rect);
	// draw text
	dc.DrawTextWithShadow(text, *sym.text_font, text_pos, font_size, stretch);
}

Image SymbolFont::getImage(double font_size, const DrawableSymbol& sym) {
	if (!sym.symbol) return Image(1,1);
	if (sym.draw_text.empty() || !sym.symbol->text_font) return sym.symbol->getImage(*this, font_size);
	// with text
	Bitmap bmp(sym.symbol->getImage(*this, font_size));
	// memory dc to work with
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	RealRect sym_rect(0,0,bmp.GetWidth(),bmp.GetHeight());
	RotatedDC rdc(dc, 0, sym_rect, 1, QUALITY_AA);
	// subtract margins from size
	sym_rect.x      += font_size * sym.symbol->text_margin_left;
	sym_rect.y      += font_size * sym.symbol->text_margin_top;
	sym_rect.width  -= font_size * (sym.symbol->text_margin_left + sym.symbol->text_margin_right);
	sym_rect.height -= font_size * (sym.symbol->text_margin_top  + sym.symbol->text_margin_bottom);
	// setup text, shrink it
	double size = font_size * sym.symbol->text_font->size;
	double stretch = 1.0;
	RealSize ts;
	while (true) {
		if (size <= 0) return sym.symbol->getImage(*this, font_size); // text too small
		rdc.SetFont(*sym.symbol->text_font, size / sym.symbol->text_font->size);
		ts = rdc.GetTextExtent(sym.draw_text);
		if (ts.height <= sym_rect.height) {
			if (ts.width <= sym_rect.width) {
				break; // text fits
			} else if (ts.width * sym.symbol->text_font->max_stretch <= sym_rect.width) {
				stretch = sym_rect.width / ts.width;
				ts.width = sym_rect.width; // for alignment
				break;
			}
		}
		// text doesn't fit
		size -= rdc.getFontSizeStep();
	}
	// align text
	RealPoint text_pos = align_in_rect(sym.symbol->text_alignment, ts, sym_rect);
	// draw text
	rdc.DrawTextWithShadow(sym.draw_text, *sym.symbol->text_font, text_pos, font_size, stretch);
	// done
	dc.SelectObject(wxNullBitmap);
	return bmp.ConvertToImage();
}

// ----------------------------------------------------------------------------- : SymbolFont : sizes

void SymbolFont::getCharInfo(RotatedDC& dc, Context& ctx, double font_size, const String& text, vector<CharInfo>& out) {
	SplitSymbols symbols;
	update(ctx);
	split(text, symbols);
	getCharInfo(dc, font_size, symbols, out);
}

void SymbolFont::getCharInfo(RotatedDC& dc, double font_size, const SplitSymbols& text, vector<CharInfo>& out) {
	FOR_EACH_CONST(sym, text) {
		size_t count = sym.text.size();
		RealSize size = dc.trInvS(symbolSize(dc.trS(font_size), sym));
		size.width /= count; // divide into count parts
		for (size_t i = 0 ; i < count ; ++i) {
			out.push_back(CharInfo(size, i == count - 1 ? BREAK_MAYBE : BREAK_NO));
		}
	}
}

RealSize SymbolFont::symbolSize(double font_size, const DrawableSymbol& sym) {
	if (sym.symbol) {
		return add_diagonal(sym.symbol->size(*this, font_size), spacing);
	} else {
		return defaultSymbolSize(font_size);
	}
}

RealSize SymbolFont::defaultSymbolSize(double font_size) {
	SymbolInFont* def = defaultSymbol();
	if (def) {
		return add_diagonal(def->size(*this, font_size), spacing);
	} else {
		return add_diagonal(RealSize(1,1), spacing);
	}
}


// ----------------------------------------------------------------------------- : InsertSymbolMenu

wxMenu* SymbolFont::insertSymbolMenu(Context& ctx) {
	if (!processed_insert_symbol_menu && insert_symbol_menu) {
		update(ctx);
		// Make menu
		processed_insert_symbol_menu = insert_symbol_menu->makeMenu(ID_INSERT_SYMBOL_MENU_MIN, *this);
	}
	return processed_insert_symbol_menu;
}

String SymbolFont::insertSymbolCode(int menu_id) const {
	// find item
	if (insert_symbol_menu) {
		return insert_symbol_menu->getCode(menu_id - ID_INSERT_SYMBOL_MENU_MIN, *this);
	} else {
		return wxEmptyString;
	}
}


InsertSymbolMenu::InsertSymbolMenu()
	: type(ITEM_CODE)
{}

int InsertSymbolMenu::size() const {
	if (type == ITEM_CODE || type == ITEM_CUSTOM) {
		return 1;
	} else if (type == ITEM_SUBMENU) {
		int count = 0;
		FOR_EACH_CONST(i, items) {
			count += i->size();
		}
		return count;
	} else {
		return 0;
	}
}
String InsertSymbolMenu::getCode(int id, const SymbolFont& font) const {
	if (type == ITEM_SUBMENU) {
		FOR_EACH_CONST(i, items) {
			int id2 = id - i->size();
			if (id2 < 0) {
				return i->getCode(id, font);
			}
			id = id2;
		}
	} else if (id == 0 && type == ITEM_CODE) {
		return name;
	} else if (id == 0 && type == ITEM_CUSTOM) {
		String caption = tr(font,_("title"),   name, capitalize_sentence);
		String message = tr(font,_("message"), name, capitalize_sentence);
		return wxGetTextFromUser(message, caption);
	}
	return wxEmptyString;
}

wxMenu* InsertSymbolMenu::makeMenu(int id, SymbolFont& font) const {
	if (type == ITEM_SUBMENU) {
		wxMenu* menu = new wxMenu();
		FOR_EACH_CONST(i, items) {
			menu->Append(i->makeMenuItem(menu, id, font));
			id += i->size();
		}
		return menu;
	}
	return nullptr;
}
wxMenuItem* InsertSymbolMenu::makeMenuItem(wxMenu* parent, int first_id, SymbolFont& font) const {
	wxString menu_name = tr(font, _("menu item"), name, capitalize);
	// ensure that there is not actually an accelerator string,
	menu_name.Replace(_("\t "),_("\t"));
	#ifdef __WXMSW__
		menu_name.Replace(_("\t"),_("\t ")); // by prepending " "
	#else
		menu_name.Replace(_("\t"),_("   ")); // by simply dropping the \t
	#endif
	if (type == ITEM_SUBMENU) {
		wxMenuItem* item = new wxMenuItem(parent, wxID_ANY, menu_name,
		                                  wxEmptyString, wxITEM_NORMAL,
		                                  makeMenu(first_id, font));
		item->SetBitmap(wxNullBitmap);
		return item;
	} else if (type == ITEM_LINE) {
		wxMenuItem* item = new wxMenuItem(parent, wxID_SEPARATOR);
		return item;
	} else {
		wxMenuItem* item = new wxMenuItem(parent, first_id, menu_name);
		// Generate bitmap for use on this item
		SymbolInFont* symbol = nullptr;
		if (type == ITEM_CUSTOM) {
			symbol = font.defaultSymbol();
		} else {
			FOR_EACH(sym, font.symbols) {
				if (!sym->code.empty() && sym->enabled && name == sym->code) { 
					symbol = sym.get();
					break;
				}
			}
		}
		if (symbol) {
			item->SetBitmap(symbol->getBitmap(font, wxSize(16,16)));
		} else {
			item->SetBitmap(wxNullBitmap);
		}
		return item;
	}
}


IMPLEMENT_REFLECTION_ENUM(MenuItemType) {
	VALUE_N("code",		ITEM_CODE);
	VALUE_N("custom",	ITEM_CUSTOM);
	VALUE_N("line",		ITEM_LINE);
	VALUE_N("submenu",	ITEM_SUBMENU);
}

IMPLEMENT_REFLECTION_NO_GET_MEMBER(InsertSymbolMenu) {
	if (!items.empty() || (tag.reading() && tag.isComplex())) {
		// complex values are groups
		REFLECT(type);
		REFLECT(name);
		REFLECT(items);
		if (!items.empty()) type = ITEM_SUBMENU;
	} else {
		REFLECT_NAMELESS(name);
	}
}
template <> void GetDefaultMember::handle(const InsertSymbolMenu& m) {
	handle(m.name);
}
template <> void GetMember::handle(const InsertSymbolMenu& m) {
	handle(_("type"),  m.type);
	handle(_("name"),  m.name);
	handle(_("items"), m.items);
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
	bool changes = false;
	if (name.update(ctx)) {
		// font name changed, load another font
		loadFont(ctx);
		changes = true;
	} else if (!font) {
		loadFont(ctx);
	}
	changes |= size.update(ctx);
	changes |= alignment.update(ctx);
	return changes;
}
void SymbolFontRef::initDependencies(Context& ctx, const Dependency& dep) const {
	name.initDependencies(ctx, dep);
	size.initDependencies(ctx, dep);
	alignment.initDependencies(ctx, dep);
}

void SymbolFontRef::loadFont(Context& ctx) {
	if (name().empty()) {
		font = SymbolFontP();
	} else {
		font = SymbolFont::byName(name);
		if (name().GetChar(0) != _(' ')) {
			// ensure the dependency on the font is present in the stylesheet this ref is in
			// Getting this stylesheet from the context is a bit of a hack
			// If the name starts with a ' ', no dependency is needed;
			//  this is for packages selected with a PackageChoiceList
			StyleSheetP stylesheet = from_script<StyleSheetP>(ctx.getVariable(_("stylesheet")));
			stylesheet->requireDependency(font.get());
		}
	}
}

IMPLEMENT_REFLECTION(SymbolFontRef) {
	REFLECT(name);
	REFLECT(size);
	REFLECT(scale_down_to);
	REFLECT(alignment);
}
