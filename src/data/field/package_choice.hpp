//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <data/font.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Packaged);

// ----------------------------------------------------------------------------- : PackageChoiceField

DECLARE_POINTER_TYPE(PackageChoiceField);
DECLARE_POINTER_TYPE(PackageChoiceStyle);
DECLARE_POINTER_TYPE(PackageChoiceValue);

/// A field for PackageChoice values, it contains a list of choices for PackageChoices
class PackageChoiceField : public Field {
public:
  PackageChoiceField() : required(true), empty_name(_("none")) {}
  DECLARE_FIELD_TYPE(PackageChoice);
  
  OptionalScript     script;      ///< Script to apply to all values
  String             match;       ///< Package filenames to match
  String             initial;     ///< Initial value
  bool               required;    ///< Is selecting a package required?
  String             empty_name;  ///< Displayed name for the empty value (if !required)
  
  void initDependencies(Context&, const Dependency&) const override;
};

// ----------------------------------------------------------------------------- : PackageChoiceStyle

/// The Style for a PackageChoiceField
class PackageChoiceStyle : public Style {
public:
  PackageChoiceStyle(const PackageChoiceFieldP& field);
  DECLARE_STYLE_TYPE(PackageChoice);
  
  Font font;  ///< Font to use for the text
  
  int update(Context&) override;
};

// ----------------------------------------------------------------------------- : PackageChoiceValue

/// The Value in a PackageChoiceField
class PackageChoiceValue : public Value {
public:
  PackageChoiceValue(const PackageChoiceFieldP& field) : Value(field), package_name(field->initial) {}
  DECLARE_VALUE_TYPE(PackageChoice, String);
  
  ValueType package_name;  ///< The selected package
  
  /// Get the package (if it is set), otherwise return nullptr
  PackagedP getPackage() const;
  
  bool update(Context&) override;
};

