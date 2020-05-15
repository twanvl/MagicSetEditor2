//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <util/rotation.hpp>
#include <util/age.hpp>
#include <data/field.hpp>
#include <data/font.hpp>
#include <data/symbol_font.hpp>
#include <script/scriptable.hpp>
#include <script/image.hpp>
#include <gfx/gfx.hpp>

// ----------------------------------------------------------------------------- : TextField

DECLARE_POINTER_TYPE(TextField);
DECLARE_POINTER_TYPE(TextStyle);
DECLARE_POINTER_TYPE(TextValue);
DECLARE_POINTER_TYPE(TextBackground);
DECLARE_POINTER_TYPE(TextLayout);
DECLARE_POINTER_TYPE(LineLayout);

/// A field for values containing tagged text
class TextField : public Field {
public:
  TextField();
  DECLARE_FIELD_TYPE(Text);
  
  OptionalScript script;          ///< Script to apply to all values
  OptionalScript default_script;  ///< Script that generates the default value
  //%OptionalScript view_script;    ///< Script to apply before viewing
  //%OptionalScript unview_script;  ///< Script to apply after changes to the view
  bool multi_line;                ///< Are newlines allowed in the text?
  String default_name;            ///< Name of "default" value
  
  void initDependencies(Context&, const Dependency&) const override;
};

// ----------------------------------------------------------------------------- : TextStyle

// information coming from text rendering
class LineLayout : public IntrusivePtrVirtualBase {
public:
  double width, top, height;
  enum class Type { LINE, PARAGRAPH, BLOCK, ALL } type;
  vector<LineLayoutP> lines, paragraphs, blocks;

  LineLayout() {}
  LineLayout(double width, double top, double height, Type type) : width(width), top(top), height(height), type(type) {}
  inline double bottom() const { return top+height; }
  void reflect(GetMember& gm) const;
};

class TextLayout : public LineLayout {
public:
  vector<double> separators;
  TextLayout() : LineLayout(0,0,0,Type::ALL) {}
  void reflect(GetMember& gm) const;
};

/// The Style for a TextField
class TextStyle : public Style {
public:
  TextStyle(const TextFieldP&);
  DECLARE_STYLE_TYPE(Text);
  
  Font font;                                  ///< Font to use for the text
  SymbolFontRef symbol_font;                  ///< Symbol font for symbols in the text
  bool always_symbol;                         ///< Should everything be drawn as symbols?
  bool allow_formating;                       ///< Is formating (bold/italic/..) allowed?
  Scriptable<Alignment> alignment;            ///< Alignment inside the box
  Scriptable<double>
      padding_left,   padding_left_min,       ///< Padding
      padding_right,  padding_right_min,      ///< Padding
      padding_top,    padding_top_min,        ///< Padding
      padding_bottom, padding_bottom_min,     ///< Padding
      line_height_soft,                       ///< Line height for soft linebreaks
      line_height_hard,                       ///< Line height for hard linebreaks
      line_height_line,                       ///< Line height for <line> tags
      line_height_soft_max,                   ///< Maximum line height
      line_height_hard_max,                   ///< Maximum line height
      line_height_line_max,                   ///< Maximum line height
      paragraph_height;                       ///< Fixed height of paragraphs
  Direction direction;                        ///< In what direction is text layed out?
  // information from text rendering
  TextLayoutP layout;
  double content_width, content_height;       ///< Size of the rendered text
  int    content_lines;                       ///< Number of rendered lines
  
  int  update(Context&) override;
  void initDependencies(Context&, const Dependency&) const override;
  void checkContentDependencies(Context&, const Dependency&) const override;
  
  /// Stretch factor to use
  double getStretch() const;
};

// ----------------------------------------------------------------------------- : TextValue

/// The Value in a TextField
class TextValue : public Value {
public:
  inline TextValue(const TextFieldP& field) : Value(field), last_update(1) {}
  DECLARE_VALUE_TYPE(Text, Defaultable<String>);
  
  ValueType value;                ///< The text of this value
  Age       last_update;          ///< When was the text last changed?
  
  bool update(Context&) override;
};

// ----------------------------------------------------------------------------- : TextValue

/// A 'fake' TextValue that is used to edit some other string
/** Used by TextCtrl */
class FakeTextValue : public TextValue {
public:
  /// Initialize the fake text value
  /** underlying can be nullptr, in that case there is no underlying value */
  FakeTextValue(const TextFieldP& field, String* underlying, bool editable, bool untagged);
  
  String* const underlying; ///< The underlying actual value, can be null
  bool const editable;      ///< The underlying value can be edited
  bool const untagged;      ///< The underlying value is untagged
  
  /// Store the value in the underlying value.
  /** May be overloaded to do some transformation */
  virtual void store();
  /// Retrieve the value from the underlying value.
  /** May be overloaded to do some transformation */
  virtual void retrieve();
  
  /// Update underlying data
  void onAction(Action& a, bool undone) override;
  /// Editing the same underlying value?
  bool equals(const Value* that) override;
};

