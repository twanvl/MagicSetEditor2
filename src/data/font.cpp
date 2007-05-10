//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Font

Font::Font()
	: name()
	, size(1)
	, underline(false)
	, weight_i(wxFONTWEIGHT_NORMAL), style_i(wxFONTSTYLE_NORMAL)
	, scale_down_to(100000)
	, shadow_displacement(0,0)
	, separator_color(128,128,128)
	, type(NORMAL)
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
	weight_i  =  weight() == _("bold")   ? wxBOLD   : wxNORMAL;
	style_i   =  style()  == _("italic") ? wxITALIC : wxNORMAL;
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

FontP Font::make(bool bold, bool italic, bool placeholder_color, bool code_color, Color* other_color) const {
	FontP f(new Font(*this));
	if (bold)   f->weight_i = wxFONTWEIGHT_BOLD;
	if (italic) f->style_i  = wxFONTSTYLE_ITALIC;
	if (code_color) {
		f->color = Color(128,0,0);
		f->type  = TYPEWRITER;
	}
	if (placeholder_color) {
		f->color = f->separator_color;
		f->shadow_displacement = RealSize(0,0); // no shadow
	}
	if (other_color) {
		f->color = *other_color;
	}
	return f;
}

wxFont Font::toWxFont(double scale) const {
	int size_i = scale * size;
	if (name().empty()) {
		wxFont font = *wxNORMAL_FONT;
		font.SetPointSize(size > 1 ? size_i : scale * font.GetPointSize());
		return font;
	} else if (type == TYPEWRITER) {
		return wxFont(size_i, wxFONTFAMILY_TELETYPE, weight_i, underline(), _("Courier New"));
	} else if (style_i == wxFONTSTYLE_ITALIC && !italic_name().empty()) {
		return wxFont(size_i, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, weight_i, underline(), italic_name());
	} else {
		return wxFont(size_i, wxFONTFAMILY_DEFAULT, style_i, weight_i, underline(), name());
	}
}

IMPLEMENT_REFLECTION(Font) {
	REFLECT(name);
	REFLECT(size);
	REFLECT(weight);
	REFLECT(style);
	REFLECT(underline);
	REFLECT(italic_name);
	REFLECT(color);
	REFLECT(scale_down_to);
	REFLECT_N("shadow_displacement_x", shadow_displacement.width);
	REFLECT_N("shadow_displacement_y", shadow_displacement.height);
	REFLECT(shadow_color);
	REFLECT(separator_color);
}
