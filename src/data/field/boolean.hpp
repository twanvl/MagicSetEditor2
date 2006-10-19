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

/// A field whos value is either true or false
class BooleanField : public ChoiceField {
  public:
	BooleanField();
	
	// no extra data
	
	virtual ValueP newValue(const FieldP& thisP) const;
	virtual StyleP newStyle(const FieldP& thisP) const;
	virtual String typeName() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : BooleanStyle

/// The Style for a BooleanField
class BooleanStyle : public ChoiceStyle {
  public:
	// no extra data
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : BooleanValue

/// The Value in a BooleanField
class BooleanValue : public ChoiceValue {
  public:
	// no extra data
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
