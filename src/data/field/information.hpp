//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_INFORMATION
#define HEADER_DATA_FIELD_INFORMATION

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
	InfoField() {}
	DECLARE_FIELD_TYPE(Text);
	
	OptionalScript script;			///< Script to apply to all values
	
	virtual void initDependencies(Context&, const Dependency&) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : InfoStyle

/// The Style for a InfoField
class InfoStyle : public Style {
  public:
	InfoStyle(const InfoFieldP&);
	DECLARE_STYLE_TYPE(Info);
	
	Font font;									///< Font to use for the text
	Alignment alignment;						///< Alignment inside the box
	double padding_left, padding_right;			///< Padding
	double padding_top, padding_bottom;
	Color background_color;
	
	virtual bool update(Context&);
	virtual void initDependencies(Context&, const Dependency&) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : InfoValue

/// The Value in a InfoField
class InfoValue : public Value {
  public:
	inline InfoValue(const InfoFieldP& field) : Value(field) {}
	DECLARE_HAS_FIELD(Info)
	
	String value;
	
	virtual String toString() const;
	virtual bool update(Context&);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
