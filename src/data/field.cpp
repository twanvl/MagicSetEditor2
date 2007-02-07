//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/field/boolean.hpp>
#include <data/field/image.hpp>
#include <data/field/symbol.hpp>
#include <data/field/color.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Field

Field::Field()
	: index            (0) // sensible default?
	, editable         (true)
	, save_value       (true)
	, show_statistics  (true)
	, identifying      (false)
	, card_list_column (100)
	, card_list_width  (100)
	, card_list_visible(false)
	, card_list_allow  (true)
	, card_list_align  (ALIGN_LEFT)
	, tab_index        (0)
{}

Field::~Field() {}

IMPLEMENT_REFLECTION(Field) {
	if (!tag.reading()) {
		String type = typeName();
		REFLECT(type);
	}
	REFLECT(name);
	if (tag.reading()) name = cannocial_name_form(name);
	REFLECT(description);
	REFLECT_N("icon", icon_filename);
	REFLECT(editable);
	REFLECT(save_value);
	REFLECT(show_statistics);
	REFLECT(identifying);
	REFLECT(card_list_column);
	REFLECT(card_list_width);
	REFLECT(card_list_visible);
	REFLECT(card_list_allow);
	REFLECT(card_list_name);
	if (tag.reading() && card_list_name.empty()) card_list_name = name;
	REFLECT_N("card_list_alignment", card_list_align);
	REFLECT(tab_index);
}

template <>
shared_ptr<Field> read_new<Field>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
	if      (type == _("text"))				return new_shared<TextField>();
	else if (type == _("choice"))			return new_shared<ChoiceField>();
	else if (type == _("multiple choice"))	return new_shared<MultipleChoiceField>();
	else if (type == _("boolean"))			return new_shared<BooleanField>();
	else if (type == _("image"))			return new_shared<ImageField>();
	else if (type == _("symbol"))			return new_shared<SymbolField>();
	else if (type == _("color"))			return new_shared<ColorField>();
	else {
		throw ParseError(_("Unsupported field type: '") + type + _("'"));
	}
}

// ----------------------------------------------------------------------------- : Style

Style::Style(const FieldP& field)
	: fieldP(field)
	, z_index(0)
	, left(0),  top(0)
	, width(0), height(0)
	, visible(true)
{}

Style::~Style() {}

IMPLEMENT_REFLECTION(Style) {
	REFLECT(z_index);
	REFLECT(left);
	REFLECT(top);
	REFLECT(width);
	REFLECT(height);
	REFLECT(visible);
}

void init_object(const FieldP& field, StyleP& style) {
	if (!style) style = field->newStyle(field);
}
template <> StyleP read_new<Style>(Reader&) {
	throw InternalError(_("IndexMap contains nullptr StyleP the application should have crashed already"));
}

bool Style::update(Context& ctx) {
	return left   .update(ctx)
	     | top    .update(ctx)
	     | width  .update(ctx)
	     | height .update(ctx)
	     | visible.update(ctx);
}

void Style::initDependencies(Context& ctx, const Dependency& dep) const {
///	left   .initDependencies(ctx,dep);
//	top    .initDependencies(ctx,dep);
//	width  .initDependencies(ctx,dep);
//	height .initDependencies(ctx,dep);
//	visible.initDependencies(ctx,dep);
}

// ----------------------------------------------------------------------------- : Value

Value::~Value() {}

IMPLEMENT_REFLECTION_NAMELESS(Value) {
}

void init_object(const FieldP& field, ValueP& value) {
	if (!value) value = field->newValue(field);
}
template <> ValueP read_new<Value>(Reader&) {
	throw InternalError(_("IndexMap contains nullptr ValueP the application should have crashed already"));
}

