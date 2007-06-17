//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
	REFLECT(empty_choice);
}

// ----------------------------------------------------------------------------- : MultipleChoiceStyle

MultipleChoiceStyle::MultipleChoiceStyle(const MultipleChoiceFieldP& field)
	: ChoiceStyle(field)
	, direction(LEFT_TO_RIGHT)
	, spacing(0)
{}

IMPLEMENT_REFLECTION(MultipleChoiceStyle) {
	REFLECT_BASE(ChoiceStyle);
	REFLECT(direction);
	REFLECT(spacing);
}

// ----------------------------------------------------------------------------- : MultipleChoiceValue

IMPLEMENT_REFLECTION_NAMELESS(MultipleChoiceValue) {
	REFLECT_BASE(ChoiceValue);
}

bool MultipleChoiceValue::update(Context& ctx) {
	String old_value = value();
	ctx.setVariable(_("last change"), to_script(last_change));
	ChoiceValue::update(ctx);
	normalForm();
	return value() != old_value;
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

void MultipleChoiceValue::normalForm() {
	String& val = value.mutateDontChangeDefault();
	// which choices are active?
	vector<bool> seen(field().choices->lastId());
	for (size_t pos = 0 ; pos < val.size() ; ) {
		if (val.GetChar(pos) == _(' ')) {
			++pos; // ingore whitespace
		} else {
			// does this choice match the one asked about?
			size_t end = val.find_first_of(_(','), pos);
			if (end == String::npos) end = val.size();
			// find this choice
			for (size_t i = 0 ; i < seen.size() ; ++i) {
				if (is_substr(val, pos, field().choices->choiceName((int)i))) {
					seen[i] = true;
					break;
				}
			}
			pos = end + 1;
		}
	}
	// now put them back in the right order
	val.clear();
	for (size_t i = 0 ; i < seen.size() ; ++i) {
		if (seen[i]) {
			if (!val.empty()) val += _(", ");
			val += field().choices->choiceName((int)i);
		}
	}
	// empty choice name
	if (val.empty()) {
		val = field().empty_choice;
	}
}
