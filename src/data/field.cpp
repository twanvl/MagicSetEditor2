//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field.hpp>

// ----------------------------------------------------------------------------- : Field

IMPLEMENT_REFLECTION(Field) {
}

template <>
shared_ptr<Field> read_new<Field>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
//	if (type == _("text")) {
//	} else {
		throw "TODO";
//	}
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

