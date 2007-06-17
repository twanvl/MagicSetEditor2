//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
	return format_string(_ACTION_("change"), valueP->fieldP->name);
}

// ----------------------------------------------------------------------------- : Simple

/// Swap the value in a Value object with a new one
inline void swap_value(ChoiceValue&         a, ChoiceValue        ::ValueType& b) { swap(a.value,    b); }
inline void swap_value(ColorValue&          a, ColorValue         ::ValueType& b) { swap(a.value,    b); }
inline void swap_value(ImageValue&          a, ImageValue         ::ValueType& b) { swap(a.filename, b); a.last_update.update(); }
inline void swap_value(SymbolValue&         a, SymbolValue        ::ValueType& b) { swap(a.filename, b); a.last_update.update(); }
inline void swap_value(TextValue&           a, TextValue          ::ValueType& b) { swap(a.value,    b); a.last_update.update(); }
inline void swap_value(MultipleChoiceValue& a, MultipleChoiceValue::ValueType& b) {
	swap(a.value,       b.value);
	swap(a.last_change, b.last_change);
}

/// A ValueAction that swaps between old and new values
template <typename T, bool ALLOW_MERGE>
class SimpleValueAction : public ValueAction {
  public:
	inline SimpleValueAction(const intrusive_ptr<T>& value, const typename T::ValueType& new_value)
		: ValueAction(value), new_value(new_value)
	{}
	
	virtual void perform(bool to_undo) {
		swap_value(static_cast<T&>(*valueP), new_value);
		valueP->onAction(*this, to_undo); // notify value
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

ValueAction* value_action(const ChoiceValueP&         value, const Defaultable<String>& new_value) { return new SimpleValueAction<ChoiceValue,         true> (value, new_value); }
ValueAction* value_action(const ColorValueP&          value, const Defaultable<Color>&  new_value) { return new SimpleValueAction<ColorValue,          true> (value, new_value); }
ValueAction* value_action(const ImageValueP&          value, const FileName&            new_value) { return new SimpleValueAction<ImageValue,          false>(value, new_value); }
ValueAction* value_action(const SymbolValueP&         value, const FileName&            new_value) { return new SimpleValueAction<SymbolValue,         false>(value, new_value); }
ValueAction* value_action(const MultipleChoiceValueP& value, const Defaultable<String>& new_value, const String& last_change) {
	MultipleChoiceValue::ValueType v = { new_value, last_change };
	return new SimpleValueAction<MultipleChoiceValue, false>(value, v);
}


// ----------------------------------------------------------------------------- : Text

TextValueAction::TextValueAction(const TextValueP& value, size_t start, size_t end, size_t new_end, const Defaultable<String>& new_value, const String& name)
	: ValueAction(value)
	, selection_start(start), selection_end(end), new_selection_end(new_end)
	, new_value(new_value)
	, name(name)
{}

String TextValueAction::getName(bool to_undo) const { return name; }

void TextValueAction::perform(bool to_undo) {
	swap_value(value(), new_value);
	swap(selection_end, new_selection_end);
	valueP->onAction(*this, to_undo); // notify value
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


TextValueAction* toggle_format_action(const TextValueP& value, const String& tag, size_t start_i, size_t end_i, size_t start, size_t end, const String& action_name) {
	if (start > end) {
		swap(start, end);
		swap(start_i, end_i);
	}
	String new_value;
	const String& str = value->value();
	// Are we inside the tag we are toggling?
	size_t tagpos = in_tag(str, _("<") + tag, start_i, end_i);
	if (tagpos == String::npos) {
		// we are not inside this tag, add it
		new_value =  str.substr(0, start_i);
		new_value += _("<") + tag + _(">");
		new_value += str.substr(start_i, end_i - start_i);
		new_value += _("</") + tag + _(">");
		new_value += str.substr(end_i);
	} else {
		// we are inside this tag, _('remove') it
		new_value =  str.substr(0, start_i);
		new_value += _("</") + tag + _(">");
		new_value += str.substr(start_i, end_i - start_i);
		new_value += _("<") + tag + _(">");
		new_value += str.substr(end_i);
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

TextValueAction* typing_action(const TextValueP& value, size_t start_i, size_t end_i, size_t start, size_t end, const String& replacement, const String& action_name)  {
	bool reverse = start > end;
	if (reverse) {
		swap(start, end);
		swap(start_i, end_i);
	}
	String new_value = tagged_substr_replace(value->value(), start_i, end_i, replacement);
	if (value->value() == new_value) {
		// no change
		return nullptr;
	} else {
		if (reverse) {
			return new TextValueAction(value, end, start, start+untag(replacement).size(), new_value, action_name);
		} else {
			return new TextValueAction(value, start, end, start+untag(replacement).size(), new_value, action_name);
		}
	}
}

// ----------------------------------------------------------------------------- : Reminder text

TextToggleReminderAction::TextToggleReminderAction(const TextValueP& value, size_t pos_in)
	: ValueAction(value)
{
	pos = in_tag(value->value(), _("<kw-"), pos_in, pos_in);
	if (pos == String::npos) {
		throw InternalError(_("TextToggleReminderAction: not in <kw- tag"));
	}
	Char c = value->value().GetChar(pos + 4);
	enable = !(c == _('1') || c == _('A')); // if it was not enabled, then enable it
	old = enable ? _('1') : _('0');
}
String TextToggleReminderAction::getName(bool to_undo) const {
	return enable ? _("Show reminder text") : _("Hide reminder text");
}

void TextToggleReminderAction::perform(bool to_undo) {
	TextValue& value = static_cast<TextValue&>(*valueP);
	String& val = value.value.mutate();
	assert(pos + 4 < val.size());
	size_t end = match_close_tag(val, pos);
	Char& c = val[pos + 4];
	swap(c, old);
	if (end != String::npos && end + 5 < val.size()) {
		val[end + 5] = c; // </kw-c>
	}
	value.last_update.update();
	value.onAction(*this, to_undo); // notify value
}


// ----------------------------------------------------------------------------- : Event

String ScriptValueEvent::getName(bool) const {
	assert(false); // this action is just an event, getName shouldn't be called
	throw InternalError(_("ScriptValueEvent::getName"));
}
void ScriptValueEvent::perform(bool) {
	assert(false); // this action is just an event, it should not be performed
}


String ScriptStyleEvent::getName(bool) const {
	assert(false); // this action is just an event, getName shouldn't be called
	throw InternalError(_("ScriptStyleEvent::getName"));
}
void ScriptStyleEvent::perform(bool) {
	assert(false); // this action is just an event, it should not be performed
}
