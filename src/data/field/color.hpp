//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_COLOR
#define HEADER_DATA_FIELD_COLOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>
#include <script/image.hpp>

// ----------------------------------------------------------------------------- : ColorField

DECLARE_POINTER_TYPE(ColorField);
DECLARE_POINTER_TYPE(ColorStyle);
DECLARE_POINTER_TYPE(ColorValue);

/// A field for color values, it contains a list of choices for colors
class ColorField : public Field {
  public:
	ColorField();
	DECLARE_FIELD_TYPE(Color);
	
	class Choice;
	typedef intrusive_ptr<Choice> ChoiceP;
	
	OptionalScript     script;			///< Script to apply to all values
	OptionalScript     default_script;	///< Script that generates the default value
	vector<ChoiceP>    choices;			///< Color choices available
	bool               allow_custom;	///< Are colors not in the list of choices allowed?
	Defaultable<Color> initial;			///< Initial choice of a new value, if not set the first choice is used
	String             default_name;	///< Name of "default" value
	
	virtual void initDependencies(Context&, const Dependency&) const;
};

/// A color that can be chosen for this field
class ColorField::Choice : public IntrusivePtrBase<ColorField::Choice> {
  public:
	String name;		///< Name of the color
	Color  color;		///< The actual color
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ColorStyle

/// The Style for a ColorField
class ColorStyle : public Style {
  public:
	ColorStyle(const ColorFieldP& field);
	DECLARE_STYLE_TYPE(Color);
	
	double       radius;          ///< Radius of round corners
	double       left_width;      ///< Width of the colored region on the left side
	double       right_width;     ///< Width of the colored region on the right side
	double       top_width;       ///< Width of the colored region on the top side
	double       bottom_width;    ///< Width of the colored region on the bottom side
	ImageCombine combine;         ///< How to combine image with the background
	
	virtual int update(Context&);
};

// ----------------------------------------------------------------------------- : ColorValue

/// The Value in a ColorField
class ColorValue : public Value {
  public:
	ColorValue(const ColorFieldP& field);
	DECLARE_VALUE_TYPE(Color, Defaultable<Color>);
	
	ValueType value;	///< The value
	
	virtual bool update(Context&);
};


// ----------------------------------------------------------------------------- : EOF
#endif
