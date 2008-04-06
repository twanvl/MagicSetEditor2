//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_VALUE
#define HEADER_DATA_ACTION_VALUE

/** @file data/action/value.hpp
 *
 *  Actions operating on Values (and derived classes, "*Value")
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <util/defaultable.hpp>

class Card;
class StyleSheet;
DECLARE_POINTER_TYPE(Value);
DECLARE_POINTER_TYPE(Style);
DECLARE_POINTER_TYPE(TextValue);
DECLARE_POINTER_TYPE(ChoiceValue);
DECLARE_POINTER_TYPE(MultipleChoiceValue);
DECLARE_POINTER_TYPE(ColorValue);
DECLARE_POINTER_TYPE(ImageValue);
DECLARE_POINTER_TYPE(SymbolValue);
DECLARE_POINTER_TYPE(PackageChoiceValue);

// ----------------------------------------------------------------------------- : ValueAction (based class)

/// An Action the changes a Value
class ValueAction : public Action {
  public:
	inline ValueAction(const Card* card, const ValueP& value) : valueP(value), card(card) {}
	
	virtual String getName(bool to_undo) const;
	
	const ValueP valueP; ///< The modified value
	const Card*  card;   ///< The card the value is on, or null if it is not a card value
};

// ----------------------------------------------------------------------------- : Simple

/// Action that updates a Value to a new value
ValueAction* value_action(const Card* card, const ChoiceValueP&         value, const Defaultable<String>& new_value);
ValueAction* value_action(const Card* card, const MultipleChoiceValueP& value, const Defaultable<String>& new_value, const String& last_change);
ValueAction* value_action(const Card* card, const ColorValueP&          value, const Defaultable<Color>&  new_value);
ValueAction* value_action(const Card* card, const ImageValueP&          value, const FileName&            new_value);
ValueAction* value_action(const Card* card, const SymbolValueP&         value, const FileName&            new_value);
ValueAction* value_action(const Card* card, const PackageChoiceValueP&  value, const String&              new_value);

// ----------------------------------------------------------------------------- : Text

/// An action that changes a TextValue
class TextValueAction : public ValueAction {
  public:
	TextValueAction(const Card* card, const TextValueP& value, size_t start, size_t end, size_t new_end, const Defaultable<String>& new_value, const String& name);
	
	virtual String getName(bool to_undo) const;
	virtual void perform(bool to_undo);
	virtual bool merge(const Action& action);
	
	inline const String& newValue() const { return new_value(); }
	
	/// The modified selection
	size_t selection_start, selection_end;
  private:
	inline TextValue& value() const;
	
	size_t new_selection_end;
	Defaultable<String> new_value;
	String name;
};

/// Action for toggling some formating tag on or off in some range
TextValueAction* toggle_format_action(const Card* card, const TextValueP& value, const String& tag, size_t start_i, size_t end_i, size_t start, size_t end, const String& action_name);

/// Typing in a TextValue, replace the selection [start...end) with replacement
/** start and end are cursor positions, start_i and end_i are indices*/
TextValueAction* typing_action(const Card* card, const TextValueP& value, size_t start_i, size_t end_i, size_t start, size_t end, const String& replacement, const String& action_name);

// ----------------------------------------------------------------------------- : Reminder text

/// Toggle reminder text for a keyword on or off
class TextToggleReminderAction : public ValueAction {
  public:
	TextToggleReminderAction(const Card* card, const TextValueP& value, size_t pos);
	
	virtual String getName(bool to_undo) const;
	virtual void perform(bool to_undo);
	
  private:	
	size_t pos;  ///< Position of "<kw-"
	bool enable; ///< Should the reminder text be turned on or off?
	Char old;    ///< Old value of the <kw- tag
};

// ----------------------------------------------------------------------------- : Replace all

/// A TextValueAction without the start and end stuff
class SimpleTextValueAction : public ValueAction {
  public:
	SimpleTextValueAction(const Card* card, const TextValueP& value, const Defaultable<String>& new_value);
	virtual void perform(bool to_undo);
	bool merge(const SimpleTextValueAction& action);
  private:
	Defaultable<String> new_value;
};

/// An action from "Replace All"; just a bunch of value actions performed in sequence
class ReplaceAllAction : public Action {
  public:
	~ReplaceAllAction();
	
	virtual String getName(bool to_undo) const;
	virtual void perform(bool to_undo);
	
	vector<SimpleTextValueAction> actions;
};

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

/// Notification that a script caused a style to change
class ScriptStyleEvent : public Action {
  public:
	inline ScriptStyleEvent(const StyleSheet* stylesheet, const Style* style)
		: stylesheet(stylesheet), style(style)
	{}
		
	virtual String getName(bool to_undo) const;
	virtual void perform(bool to_undo);
	
	const StyleSheet* stylesheet; ///< StyleSheet the style is for
	const Style*      style;      ///< The modified style
};

// ----------------------------------------------------------------------------- : EOF
#endif
