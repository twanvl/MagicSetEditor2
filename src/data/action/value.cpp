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

// ----------------------------------------------------------------------------- : ValueAction

String ValueAction::getName(bool to_undo) const {
	return _("Change ") + valueP->fieldP->name;
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
	
	virtual bool merge(const Action* action) {
		if (!ALLOW_MERGE) return false;
		if (const SimpleValueAction* sva = dynamic_cast<const SimpleValueAction*>(action)) {
			if (sva->valueP == valueP) {
				// adjacent actions on the same value, discard the other one,
				// because it only keeps an intermediate value
				return true;
			}
		} else {
			return false;
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
