//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/multiple_choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceField

MultipleChoiceField::MultipleChoiceField()
	: minimum_selection(0)
	, maximum_selection(1000000)
{}

IMPLEMENT_FIELD_TYPE(MultipleChoice)

String MultipleChoiceField::typeName() const {
	return _("multiple choice");
}

IMPLEMENT_REFLECTION(MultipleChoiceField) {
	REFLECT_BASE(ChoiceField);
	REFLECT(minimum_selection);
	REFLECT(maximum_selection);
}

// ----------------------------------------------------------------------------- : MultipleChoiceStyle

MultipleChoiceStyle::MultipleChoiceStyle(const MultipleChoiceFieldP& field)
	: ChoiceStyle(field)
	, direction(HORIZONTAL)
	, spacing(0)
{}

IMPLEMENT_REFLECTION_ENUM(Direction) {
	VALUE_N("horizontal", HORIZONTAL);
	VALUE_N("vertical",   VERTICAL);
}

IMPLEMENT_REFLECTION(MultipleChoiceStyle) {
	REFLECT_BASE(ChoiceStyle);
	REFLECT(direction);
	REFLECT(spacing);
}

// ----------------------------------------------------------------------------- : MultipleChoiceValue

IMPLEMENT_REFLECTION_NAMELESS(MultipleChoiceValue) {
	REFLECT_BASE(ChoiceValue);
}

void MultipleChoiceValue::get(vector<String>& out) const {
	// split the value
	out.clear();
	bool is_new = true;
	FOR_EACH_CONST(c, value()) {
		if (c == _(',')) {
			is_new = true;
		} else if (is_new) {
			if (c != _(' ')) { // ignore whitespace after ,
				is_new = false;
				out.push_back(String(1, c));
			}
		} else {
			assert(!out.empty());
			out.back() += c;
		}
	}
}
