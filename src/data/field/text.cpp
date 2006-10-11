//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/text.hpp>

// ----------------------------------------------------------------------------- : TextField

TextField::TextField()
	: multi_line(false), move_cursor_with_sort(false)
	, default_name(_("Default"))
{}

StyleP TextField::newStyle(const FieldP& thisP) const {
	assert(thisP.get() == this);
	return new_shared<TextStyle>();
}

ValueP TextField::newValue(const FieldP& thisP) const {
	assert(thisP.get() == this);
	return new_shared<TextValue>();
}

FieldP TextField::clone() const {
	return new_shared1<TextField>(*this);
}

String TextField::typeName() const {
	return _("text");
}


IMPLEMENT_REFLECTION(TextField) {
	REFLECT_BASE(Field);
	REFLECT(multi_line);
//	REFLECT(script);
//	REFLECT_N("default", default_script);
	REFLECT(move_cursor_with_sort);
	REFLECT(default_name);
}

// ----------------------------------------------------------------------------- : TextStyle

StyleP TextStyle::clone() const {
	return new_shared1<TextStyle>(*this);
}

IMPLEMENT_REFLECTION(TextStyle) {
	REFLECT_BASE(Style);
}

// ----------------------------------------------------------------------------- : TextValue

ValueP TextValue::clone() const {
	return new_shared1<TextValue>(*this);
}

IMPLEMENT_REFLECTION(TextValue) {
	REFLECT_BASE(Value);
}
