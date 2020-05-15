//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

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
};

// ----------------------------------------------------------------------------- : BooleanStyle

/// The Style for a BooleanField
class BooleanStyle : public ChoiceStyle {
public:
  BooleanStyle(const ChoiceFieldP& field);
  DECLARE_HAS_FIELD(Boolean); // not DECLARE_STYLE_TYPE, because we use a normal ChoiceValueViewer/Editor
  StyleP clone() const override;
  
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
  ValueP clone() const override;
  
  // no extra data
  
private:
  DECLARE_REFLECTION();
};

