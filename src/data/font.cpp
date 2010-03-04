//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Font

Font::Font()
	: name()
	, size(1)
	, underline(false)
	, scale_down_to(100000)
	, max_stretch(1.0)
	, color(AColor(0,0,0))
	, shadow_displacement(0,0)
	, shadow_blur(0)
	, separator_color(AColor(0,0,0,128))
	, flags(FONT_NORMAL)
{}

bool Font::update(Context& ctx) {
	bool changes
	     = name        .update(ctx)
	     | italic_name .update(ctx)
	     | size        .update(ctx)
	     | weight      .update(ctx)
	     | style       .update(ctx)
	     | underline   .update(ctx)
	     | color       .update(ctx)
	     | shadow_color.update(ctx);
	flags = (flags & ~FONT_BOLD & ~FONT_ITALIC)
	      | (weight() == _("bold")   ? FONT_BOLD   : FONT_NORMAL)
	      | (style()  == _("italic") ? FONT_ITALIC : FONT_NORMAL);
	return changes;
}
void Font::initDependencies(Context& ctx, const Dependency& dep) const {
	name        .initDependencies(ctx, dep);
	italic_name .initDependencies(ctx, dep);
	size        .initDependencies(ctx, dep);
	weight      .initDependencies(ctx, dep);
	style       .initDependencies(ctx, dep);
	underline   .initDependencies(ctx, dep);
	color       .initDependencies(ctx, dep);
	shadow_color.initDependencies(ctx, dep);
}

FontP Font::make(int add_flags, AColor* other_color, double* other_size) const {
	FontP f(new Font(*this));
	f->flags |= add_flags;
	if (add_flags & FONT_CODE_STRING) {
		f->color = Color(0,0,100);
	}
	if (add_flags & FONT_CODE) {
		f->color = Color(128,0,0);
	}
	if (add_flags & FONT_CODE_KW) {
		f->color = Color(158,0,0);
		f->flags |= FONT_BOLD;
	}
	if (add_flags & FONT_SOFT) {
		f->color = f->separator_color;
		f->shadow_displacement = RealSize(0,0); // no shadow
	}
	if (other_color) {
		f->color = *other_color;
	}
	if (other_size) {
		f->size = *other_size;
	}
	return f;
}

wxFont Font::toWxFont(double scale) const {
	int size_i = to_int(scale * size);
	int weight_i = flags & FONT_BOLD   ? wxFONTWEIGHT_BOLD  : wxFONTWEIGHT_NORMAL;
	int style_i  = flags & FONT_ITALIC ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL;
	// make font
	wxFont font;
	if (flags & FONT_CODE) {
		if (size_i < 2) {
			return wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, weight_i, underline(), _("Courier New"));
		} else {
			font = wxFont(size_i, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, weight_i, underline(), _("Courier New"));
		}
	} else if (name().empty()) {
		font = *wxNORMAL_FONT;
		font.SetPointSize(size > 1 ? size_i : int(scale * font.GetPointSize()));
		return font;
	} else if (flags & FONT_ITALIC && !italic_name().empty()) {
		font = wxFont(size_i, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, weight_i, underline(), italic_name());
	} else {
		font = wxFont(size_i, wxFONTFAMILY_DEFAULT, style_i, weight_i, underline(), name());
	}
	// fix size
	#ifdef __WXMSW__
		// make it independent of screen dpi, always use 96 dpi
		// TODO: do something more sensible, and more portable
		font.SetPixelSize(wxSize(0, -(int)(scale*size*96.0/72.0 + 0.5) ));
	#endif
	return font;
}

IMPLEMENT_REFLECTION_NO_SCRIPT(Font) {
	REFLECT(name);
	REFLECT(size);
	REFLECT(weight);
	REFLECT(style);
	REFLECT(underline);
	REFLECT(italic_name);
	REFLECT(color);
	REFLECT(scale_down_to);
	REFLECT(max_stretch);
	REFLECT_N("shadow_displacement_x", shadow_displacement.width);
	REFLECT_N("shadow_displacement_y", shadow_displacement.height);
	REFLECT(shadow_color);
	REFLECT(shadow_blur);
	REFLECT(separator_color);
}
