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

/// Utility macro for declaring classes derived from ValueAction
#define DECLARE_VALUE_ACTION(Type)						\
  protected:											\
	inline Type##Value& value() const {					\
		return static_cast<Type##Value&>(*valueP);		\
	}													\
  public:												\
	virtual void   perform(bool to_undo)


// ----------------------------------------------------------------------------- : Simple

ValueAction* value_action(const ChoiceValueP& value, const Defaultable<String>& new_value);
ValueAction* value_action(const ColorValueP&  value, const Defaultable<Color>&  new_value);
ValueAction* value_action(const ImageValueP&  value, const FileName&            new_value);
ValueAction* value_action(const SymbolValueP& value, const FileName&            new_value);

// ----------------------------------------------------------------------------- : Text

/*
class ColorValueAction : public ValueAction {
  public:
	ColorValueAction(const ColorValueP& value, const Defaultable<Color>& color);
	
	DECLARE_VALUE_ACTION(Color);
	
  private:
	Defaultable<Color> color;	///< The new/old color
};
*/

// ----------------------------------------------------------------------------- : EOF
#endif
