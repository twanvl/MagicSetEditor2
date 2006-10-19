//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
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
	, card_list_column (-1)
	, card_list_width  (100)
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
	REFLECT(editable);
	REFLECT(save_value);
	REFLECT(show_statistics);
	REFLECT(identifying);
	REFLECT(card_list_column);
	REFLECT(card_list_width);
	REFLECT(card_list_allow);
	REFLECT(card_list_align);
	REFLECT(tab_index);
}

template <>
shared_ptr<Field> read_new<Field>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
	if      (type == _("text"))		return new_shared<TextField>();
	else if (type == _("choice"))	return new_shared<ChoiceField>();
	else if (type == _("boolean"))	return new_shared<BooleanField>();
	else if (type == _("image"))	return new_shared<ImageField>();
	else if (type == _("symbol"))	return new_shared<SymbolField>();
	else if (type == _("color"))	return new_shared<ColorField>();
	else {
		throw ParseError(_("Unsupported field type: '") + type + _("'"));
	}
}



// ----------------------------------------------------------------------------- : Style

Style::~Style() {}

IMPLEMENT_REFLECTION(Style) {
	REFLECT(z_index);
	REFLECT(left);
	REFLECT(top);
	REFLECT(width);
	REFLECT(height);
	REFLECT(visible);
}

void initObject(const FieldP& field, StyleP& style) {
	style = field->newStyle(field);
}

// ----------------------------------------------------------------------------- : Value

Value::~Value() {}

IMPLEMENT_REFLECTION_NAMELESS(Value) {
}

void initObject(const FieldP& field, ValueP& value) {
	value = field->newValue(field);
}

