//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/color.hpp>
#include <script/script.hpp>

DECLARE_TYPEOF_COLLECTION(ColorField::ChoiceP);

// ----------------------------------------------------------------------------- : ColorField

ColorField::ColorField()
	: allow_custom(true)
	, default_name(_("Default"))
{}

IMPLEMENT_FIELD_TYPE(Color)

String ColorField::typeName() const {
	return _("color");
}

void ColorField::initDependencies(Context& ctx, const Dependency& dep) const {
	Field        ::initDependencies(ctx, dep);
	script        .initDependencies(ctx, dep);
	default_script.initDependencies(ctx, dep);
}


IMPLEMENT_REFLECTION(ColorField) {
	REFLECT_BASE(Field);
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(default_name);
	REFLECT(allow_custom);
	REFLECT(choices);
}

// ----------------------------------------------------------------------------- : ColorField::Choice

IMPLEMENT_REFLECTION(ColorField::Choice) {
	REFLECT(name);
	REFLECT(color);
}

// ----------------------------------------------------------------------------- : ColorStyle

ColorStyle::ColorStyle(const ColorFieldP& field)
	: Style(field)
	, radius(0)
	, left_width(100000), right_width (100000)
	, top_width (100000), bottom_width(100000)
{}

IMPLEMENT_REFLECTION(ColorStyle) {
	REFLECT_BASE(Style);
	REFLECT(radius);
	REFLECT(left_width);
	REFLECT(right_width);
	REFLECT(top_width);
	REFLECT(bottom_width);
	REFLECT_N("mask", mask_filename);
}

bool ColorStyle::update(Context& ctx) {
	return Style       ::update(ctx)
	     | mask_filename.update(ctx);
}

// ----------------------------------------------------------------------------- : ColorValue

String ColorValue::toString() const {
	if (value.isDefault()) return field().default_name;
	// is this a named color?
	FOR_EACH(c, field().choices) {
		if (value() == c->color) return c->name;
	}
	return _("<color>");
}
bool ColorValue::update(Context& ctx) {
	Value::update(ctx);
	return field().default_script.invokeOnDefault(ctx, value)
	     | field().        script.invokeOn(ctx, value);
}

IMPLEMENT_REFLECTION_NAMELESS(ColorValue) {
	if (fieldP->save_value || tag.scripting()) REFLECT_NAMELESS(value);
}
