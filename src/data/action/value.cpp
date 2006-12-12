//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/action/value.hpp>
#include <data/field.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/field/color.hpp>
#include <data/field/image.hpp>
#include <data/field/symbol.hpp>
#include <util/tagged_string.hpp>

// ----------------------------------------------------------------------------- : ValueAction

String ValueAction::getName(bool to_undo) const {
	return String::Format(_ACTION_("change"), valueP->fieldP->name);
}

// ----------------------------------------------------------------------------- : Simple

/// A ValueAction that swaps between old and new values
template <typename T, typename T::ValueType T::*member, bool ALLOW_MERGE>
class SimpleValueAction : public ValueAction {
  public:
	inline SimpleValueAction(const shared_ptr<T>& value, const typename T::ValueType& new_value)
		: ValueAction(value), new_value(new_value)
	{}
	
	virtual void perform(bool to_undo) {
		swap(static_cast<T&>(*valueP).*member, new_value);
	}
	
	virtual bool merge(const Action& action) {
		if (!ALLOW_MERGE) return false;
		TYPE_CASE(action, SimpleValueAction) {
			if (action.valueP == valueP) {
				// adjacent actions on the same value, discard the other one,
				// because it only keeps an intermediate value
				return true;
			}
		}
		return false;
	}
	
  private:
	typename T::ValueType new_value;
};

ValueAction* value_action(const ChoiceValueP& value, const Defaultable<String>& new_value) { return new SimpleValueAction<ChoiceValue, &ChoiceValue::value,    true> (value, new_value); }
ValueAction* value_action(const ColorValueP&  value, const Defaultable<Color>&  new_value) { return new SimpleValueAction<ColorValue,  &ColorValue::value,     true> (value, new_value); }
ValueAction* value_action(const ImageValueP&  value, const FileName&            new_value) { return new SimpleValueAction<ImageValue,  &ImageValue::filename,  false>(value, new_value); }
ValueAction* value_action(const SymbolValueP& value, const FileName&            new_value) { return new SimpleValueAction<SymbolValue, &SymbolValue::filename, false>(value, new_value); }


// ----------------------------------------------------------------------------- : Text

TextValueAction::TextValueAction(const TextValueP& value, size_t start, size_t end, size_t new_end, const Defaultable<String>& new_value, const String& name)
	: ValueAction(value), new_value(new_value), name(name)
	, selection_start(start), selection_end(end), new_selection_end(new_end)
{}

String TextValueAction::getName(bool to_undo) const { return name; }

void TextValueAction::perform(bool to_undo) {
	swap(value().value, new_value);
//	if (value().value.age < new_value.age) value().value.age = Age();
	swap(selection_end, new_selection_end);
}

bool TextValueAction::merge(const Action& action) {
	TYPE_CASE(action, TextValueAction) {
		if (&action.value() == &value() && action.name == name && action.selection_start == selection_end) {
			// adjacent edits, keep old value of this, it is older
			selection_end  = action.selection_end;
			return true;
		}
	}
	return false;
}

TextValue& TextValueAction::value() const {
	return static_cast<TextValue&>(*valueP);
}


TextValueAction* toggle_format_action(const TextValueP& value, const String& tag, size_t start, size_t end, const String& action_name) {
	if (start > end) swap(start, end);
	String new_value;
	const String& str = value->value();
	// Are we inside the tag we are toggling?
	size_t tagpos = in_tag(str, _("<") + tag, start, end);
	if (tagpos == String::npos) {
		// we are not inside this tag, add it
		new_value =  str.substr(0, start);
		new_value += _("<") + tag + _(">");
		new_value += str.substr(start, end - start);
		new_value += _("</") + tag + _(">");
		new_value += str.substr(end);
	} else {
		// we are inside this tag, _('remove') it
		new_value =  str.substr(0, start);
		new_value += _("</") + tag + _(">");
		new_value += str.substr(start, end - start);
		new_value += _("<") + tag + _(">");
		new_value += str.substr(end);
	}
	// Build action
	if (start != end) {
		// don't simplify if start == end, this way we insert <b></b>, allowing the
		// user to press Ctrl+B and start typing bold text
		new_value = simplify_tagged(new_value);
	}
	if (value->value() == new_value) {
		return nullptr; // no changes
	} else {
		return new TextValueAction(value, start, end, end, new_value, action_name);
	}
}

TextValueAction* typing_action(const TextValueP& value, size_t start, size_t end, const String& replacement, const String& action_name)  {
	bool reverse = start > end;
	if (reverse) swap(start, end);
	String new_value = tagged_substr_replace(value->value(), start, end, replacement);
	if (value->value() == new_value) {
		// no change
		return nullptr;
	} else {
//		if (name == _("Backspace")) {
//			// HACK: put start after end
		if (reverse) {
			return new TextValueAction(value, end, start, start+replacement.size(), new_value, action_name);
		} else {
			return new TextValueAction(value, start, end, start+replacement.size(), new_value, action_name);
		}
	}
}

