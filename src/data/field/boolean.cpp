//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/boolean.hpp>

// ----------------------------------------------------------------------------- : BooleanField

BooleanField::BooleanField() {
	choices->choices.push_back(new_intrusive1<Choice>(_("yes")));
	choices->choices.push_back(new_intrusive1<Choice>(_("no")));
	choices->initIds();
}

IMPLEMENT_FIELD_TYPE(Boolean, "boolean");

IMPLEMENT_REFLECTION(BooleanField) {
	REFLECT_BASE(Field); // NOTE: don't reflect as a ChoiceField
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(initial);
}

// ----------------------------------------------------------------------------- : BooleanStyle

BooleanStyle::BooleanStyle(const ChoiceFieldP& field)
	: ChoiceStyle(field)
{
	render_style = RENDER_BOTH;
	//choice_images[_("yes")] = ScriptableImage(_("buildin_image(\"bool_yes\")"));
	//choice_images[_("no")]  = ScriptableImage(_("buildin_image(\"bool_no\")"));
	choice_images[_("yes")] = ScriptableImage(new_intrusive1<BuiltInImage>(_("bool_yes")));
	choice_images[_("no")]  = ScriptableImage(new_intrusive1<BuiltInImage>(_("bool_no")));
}

IMPLEMENT_REFLECTION(BooleanStyle) {
	REFLECT_BASE(ChoiceStyle);
}

// ----------------------------------------------------------------------------- : BooleanValue

IMPLEMENT_REFLECTION_NAMELESS(BooleanValue) {
	REFLECT_BASE(ChoiceValue);
}
