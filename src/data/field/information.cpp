//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/information.hpp>
#include <script/script.hpp>

// ----------------------------------------------------------------------------- : InfoField

IMPLEMENT_FIELD_TYPE(Info, "info");

void InfoField::initDependencies(Context& ctx, const Dependency& dep) const {
	Field ::initDependencies(ctx, dep);
	script. initDependencies(ctx, dep);
}


IMPLEMENT_REFLECTION(InfoField) {
	REFLECT_BASE(Field);
	REFLECT(script);
}

// ----------------------------------------------------------------------------- : InfoStyle

InfoStyle::InfoStyle(const InfoFieldP& field)
	: Style(field)
	, alignment(ALIGN_TOP_LEFT)
	, padding_left  (2)
	, padding_right (2)
	, padding_top   (2)
	, padding_bottom(2)
	, background_color(255,255,255)
{}

int InfoStyle::update(Context& ctx) {
	return Style     ::update(ctx)
	     | font       .update(ctx) * CHANGE_OTHER;
}
void InfoStyle::initDependencies(Context& ctx, const Dependency& dep) const {
	Style     ::initDependencies(ctx, dep);
//	font       .initDependencies(ctx, dep);
}

IMPLEMENT_REFLECTION(InfoStyle) {
	REFLECT_BASE(Style);
	REFLECT(font);
	REFLECT(alignment);
	REFLECT(padding_left);
	REFLECT(padding_right);
	REFLECT(padding_top);
	REFLECT(padding_bottom);
	REFLECT(background_color);
}

// ----------------------------------------------------------------------------- : InfoValue

String InfoValue::toString() const {
	return value;
}
bool InfoValue::update(Context& ctx) {
	if (value.empty()) value = field().name;
	bool change = field().script.invokeOn(ctx, value);
	Value::update(ctx);
	return change;
}

IMPLEMENT_REFLECTION_NAMELESS(InfoValue) {
	// never save
}
