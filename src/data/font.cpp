//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Font

Font::Font()
	: font(*wxNORMAL_FONT)
	, size(font.GetPointSize())
	, scale_down_to(100000)
	, shadow_displacement(0,0)
	, separator_color(128,128,128)
{}

bool Font::update(Context& ctx) {
	return color       .update(ctx)
	     | shadow_color.update(ctx);
}
void Font::initDependencies(Context& ctx, const Dependency& dep) const {
	color       .initDependencies(ctx, dep);
	shadow_color.initDependencies(ctx, dep);
}

FontP Font::make(bool bold, bool italic) const {
	FontP f(new Font(*this));
	if (bold) f->font.SetWeight(wxBOLD);
	if (italic) {
		if (!italic_name.empty()) {
			f->font.SetFaceName(italic_name);
		} else {
			f->font.SetWeight(wxBOLD);
		}
	}
	return f;
}

void reflect_font(Reader& tag, Font& font) {
	String name, weight, style;
	double size = -1;
	REFLECT(name);
	REFLECT(size);
	REFLECT(weight);
	REFLECT(style);
	if (!name.empty())   font.font.SetFaceName(name);
	if (size > 0)        font.font.SetPointSize(font.size = size);
	if (!weight.empty()) font.font.SetWeight(weight == _("bold")   ? wxBOLD   : wxNORMAL);
	if (!style.empty())  font.font.SetWeight(style  == _("italic") ? wxITALIC : wxNORMAL);
}

template <typename Tag>
void reflect_font(Tag& tag, const Font& font) {
	REFLECT_N("name",   font.font.GetFaceName());
	REFLECT_N("size",   font.size);
	REFLECT_N("weight", font.font.GetWeight() == wxBOLD   ? _("bold")   : _("normal"));
	REFLECT_N("style",  font.font.GetStyle()  == wxITALIC ? _("italic") : _("normal"));
}

IMPLEMENT_REFLECTION(Font) {
	reflect_font(tag, *this);
	REFLECT(italic_name);
	REFLECT(color);
	REFLECT(scale_down_to);
	REFLECT_N("shadow_displacement_x", shadow_displacement.width);
	REFLECT_N("shadow_displacement_y", shadow_displacement.height);
	REFLECT(shadow_color);
	REFLECT(separator_color);
}
