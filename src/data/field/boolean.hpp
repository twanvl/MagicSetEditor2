//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_BOOLEAN
#define HEADER_DATA_FIELD_BOOLEAN

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/choice.hpp>

// ----------------------------------------------------------------------------- : BooleanField

DECLARE_POINTER_TYPE(BooleanField);
DECLARE_POINTER_TYPE(BooleanStyle);
DECLARE_POINTER_TYPE(BooleanValue);

/// A field whos value is either true or false
class BooleanField : public ChoiceField {
  public:
	BooleanField();
	DECLARE_FIELD_TYPE(Boolean);
	
	// no extra data
		
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : BooleanStyle

/// The Style for a BooleanField
class BooleanStyle : public ChoiceStyle {
  public:
	inline BooleanStyle(const ChoiceFieldP& field) : ChoiceStyle(field) {}
	DECLARE_STYLE_TYPE(Boolean);
	
	// no extra data
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : BooleanValue

/// The Value in a BooleanField
class BooleanValue : public ChoiceValue {
  public:
	inline BooleanValue(const ChoiceFieldP& field) : ChoiceValue(field) {}
	DECLARE_HAS_FIELD(Boolean);
	
	// no extra data
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
