//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_COLOR
#define HEADER_DATA_FIELD_COLOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>

// ----------------------------------------------------------------------------- : ColorField

/// A field for color values, it contains a list of choices for colors
class ColorField : public Field {
  public:
	ColorField();
	
	class Choice;
	typedef shared_ptr<Choice> ChoiceP;
	
	OptionalScript  script;			///< Script to apply to all values
	OptionalScript  default_script;	///< Script that generates the default value
	vector<ChoiceP> choices;		///< Color choices available
	bool            allow_custom;	///< Are colors not in the list of choices allowed?
	String          default_name;	///< Name of "default" value
	
	virtual ValueP newValue(const FieldP& thisP) const;
	virtual StyleP newStyle(const FieldP& thisP) const;
	virtual String typeName() const;
	
  private:
	DECLARE_REFLECTION();
};

/// A color that can be chosen for this field
class ColorField::Choice {
  public:
	String name;		///< Name of the color
	Color  color;		///< The actual color
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ColorStyle

/// The Style for a ColorField
class ColorStyle : public Style {
  public:
	ColorStyle();
	
	int  radius;			///< Radius of round corners
	UInt left_width;		///< Width of the colored region on the left side
	UInt right_width;		///< Width of the colored region on the right side
	UInt top_width;			///< Width of the colored region on the top side
	UInt bottom_width;		///< Width of the colored region on the bottom side
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ColorValue

/// The Value in a ColorField
class ColorValue : public Value {
  public:
	Defaultable<Color> value;	///< The value
	
	virtual String toString() const;
	
  private:
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : EOF
#endif
