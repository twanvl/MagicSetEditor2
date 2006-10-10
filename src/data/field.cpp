//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field.hpp>
#include <data/field/text.hpp>

// ----------------------------------------------------------------------------- : Field

Field::Field()
	: index          (0) // sensible default?
	, editable       (true)
	, saveValue      (true)
	, showStatistics (true)
	, identifying    (false)
	, cardListColumn (-1)
	, cardListWidth  (100)
	, cardListAllow  (true)
//	, cardListAlign  (ALIGN_LEFT)
	, tabIndex       (0)
{}

Field::~Field() {}

IMPLEMENT_REFLECTION(Field) {
	if (!tag.reading()) {
		String type = typeName();
		REFLECT_N("type", type);
	}
}

template <>
shared_ptr<Field> read_new<Field>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
	if (type == _("text")) {
		return new_shared<TextField>();
	} else {
		throw "TODO";
	}
}



// ----------------------------------------------------------------------------- : Style

IMPLEMENT_REFLECTION(Style) {
}

void initObject(const FieldP& field, StyleP& style) {
	style = field->newStyle(field);
}

// ----------------------------------------------------------------------------- : Value

IMPLEMENT_REFLECTION(Value) {
}

void initObject(const FieldP& field, ValueP& value) {
	value = field->newValue(field);
}

