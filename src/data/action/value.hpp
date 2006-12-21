//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_VALUE
#define HEADER_DATA_ACTION_VALUE

/** @file data/action/set.hpp
 *
 *  Actions operating on Values (and derived classes, "*Value")
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <util/defaultable.hpp>

class Card;
DECLARE_POINTER_TYPE(Value);
DECLARE_POINTER_TYPE(TextValue);
DECLARE_POINTER_TYPE(ChoiceValue);
DECLARE_POINTER_TYPE(ColorValue);
DECLARE_POINTER_TYPE(ImageValue);
DECLARE_POINTER_TYPE(SymbolValue);

// ----------------------------------------------------------------------------- : ValueAction (based class)

/// An Action the changes a Value
class ValueAction : public Action {
  public:
	inline ValueAction(const ValueP& value) : valueP(value) {}
	
	virtual String getName(bool to_undo) const;
	
	const ValueP valueP; ///< The modified value
};

// ----------------------------------------------------------------------------- : Simple

/// Action that updates a Value to a new value
ValueAction* value_action(const ChoiceValueP& value, const Defaultable<String>& new_value);
ValueAction* value_action(const ColorValueP&  value, const Defaultable<Color>&  new_value);
ValueAction* value_action(const ImageValueP&  value, const FileName&            new_value);
ValueAction* value_action(const SymbolValueP& value, const FileName&            new_value);

// ----------------------------------------------------------------------------- : Text

/// An action that changes a TextValue
class TextValueAction : public ValueAction {
  public:
	TextValueAction(const TextValueP& value, size_t start, size_t end, size_t new_end, const Defaultable<String>& new_value, const String& name);
	
	virtual String getName(bool to_undo) const;
	virtual void perform(bool to_undo);
	virtual bool merge(const Action& action);
	
	/// The modified selection
	size_t selection_start, selection_end;
  private:
	inline TextValue& value() const;
	
	size_t new_selection_end;
	Defaultable<String> new_value;
	String name;
};

/// Action for toggleing some formating tag on or off in some range
TextValueAction* toggle_format_action(const TextValueP& value, const String& tag, size_t start, size_t end, const String& action_name);

/// Typing in a TextValue, replace the selection [start...end) with replacement
TextValueAction* typing_action(const TextValueP& value, size_t start, size_t end, const String& replacement, const String& action_name);

// ----------------------------------------------------------------------------- : Event

/// Notification that a script caused a value to change
class ScriptValueEvent : public Action {
  public:
	inline ScriptValueEvent(const Card* card, const Value* value) : card(card), value(value) {}
		
	virtual String getName(bool to_undo) const;
	virtual void perform(bool to_undo);
	
	const Card* card;   ///< Card the value is on
	const Value* value; ///< The modified value
};

// ----------------------------------------------------------------------------- : EOF
#endif
