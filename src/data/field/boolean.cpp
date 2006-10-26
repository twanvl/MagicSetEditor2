//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/boolean.hpp>

// ----------------------------------------------------------------------------- : BooleanField

BooleanField::BooleanField() {
	choices->choices.push_back(new_shared1<Choice>(_("yes")));
	choices->choices.push_back(new_shared1<Choice>(_("no")));
	choices->initIds();
}

IMPLEMENT_FIELD_TYPE(Boolean)

String BooleanField::typeName() const {
	return _("boolean");
}

IMPLEMENT_REFLECTION(BooleanField) {
	REFLECT_BASE(Field); // NOTE: don't reflect as a ChoiceField
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(initial);
}

// ----------------------------------------------------------------------------- : BooleanStyle

IMPLEMENT_REFLECTION(BooleanStyle) {
	REFLECT_BASE(ChoiceStyle);
}

// ----------------------------------------------------------------------------- : BooleanValue

IMPLEMENT_REFLECTION_NAMELESS(BooleanValue) {
	REFLECT_BASE(ChoiceValue);
}
