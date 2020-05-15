//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <data/field.hpp>
#include <data/font.hpp>
#include <script/scriptable.hpp>

// ----------------------------------------------------------------------------- : InfoField

DECLARE_POINTER_TYPE(InfoField);
DECLARE_POINTER_TYPE(InfoStyle);
DECLARE_POINTER_TYPE(InfoValue);

/// A field for informational values.
/** These values are not editable, they are just headers, icons, labels, etc.
 */
class InfoField : public Field {
public:
  InfoField() { editable = false; }
  DECLARE_FIELD_TYPE(Text);
  
  OptionalScript script; ///< Script to apply to all values
  
  void initDependencies(Context&, const Dependency&) const override;
};

// ----------------------------------------------------------------------------- : InfoStyle

/// The Style for a InfoField
class InfoStyle : public Style {
public:
  InfoStyle(const InfoFieldP&);
  DECLARE_STYLE_TYPE(Info);
  
  Font font;                           ///< Font to use for the text
  Alignment alignment;                 ///< Alignment inside the box
  double padding_left, padding_right;  ///< Padding
  double padding_top, padding_bottom;
  Color background_color;
  
  int update(Context&) override;
  void initDependencies(Context&, const Dependency&) const override;
};

// ----------------------------------------------------------------------------- : InfoValue

/// The Value in a InfoField
class InfoValue : public Value {
public:
  inline InfoValue(const InfoFieldP& field) : Value(field) {}
  DECLARE_VALUE_TYPE(Info, String);
  
  ValueType value;
  
  bool update(Context&) override;
};

