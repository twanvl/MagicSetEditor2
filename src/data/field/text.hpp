//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_TEXT
#define HEADER_DATA_FIELD_TEXT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>

// ----------------------------------------------------------------------------- : TextField

/// A field that stores tagged text
class TextField : public Field {
  public:
	TextField();
	
//	Script script;
//	Script default_script;
	bool multi_line;			///< Are newlines allowed in the text?
	bool move_cursor_with_sort;	///< When the text is reordered by a script should the cursor position be updated?
	String default_name;		///< Name of "default" value
	
	virtual ValueP newValue(const FieldP& thisP) const;
	virtual StyleP newStyle(const FieldP& thisP) const;
	virtual FieldP clone() const;
	virtual String typeName() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : TextStyle

/// The Style for a TextField
class TextStyle : public Style {
  public:
//	FontInfo font;							///< Font to use for the text
//	SymbolFontInfo symbol_font;				///< Symbol font for symbols in the text
	bool always_symbol;						///< Should everything be drawn as symbols?
	bool allow_formatting;					///< Is formating (bold/italic/..) allowed?
//	Alignment alignment;					///< Alignment inside the box
	int angle;								///< Angle of the text inside the box
	int padding_left,   padding_left_min;	///< Padding
	int padding_right,  padding_right_min;	///< Padding
	int padding_top,    padding_top_min;	///< Padding
	int padding_bottom, padding_bottom_min;	///< Padding
	double line_height_soft;				///< Line height for soft linebreaks
	double line_height_hard;				///< Line height for hard linebreaks
	double line_height_line;				///< Line height for <line> tags
	String mask_filename;					///< Filename of the mask
//	ContourMaskP mask;						///< Mask to fit the text to (may be null)
	
	virtual StyleP clone() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : TextValue

/// The Value in a TextField
class TextValue : public Value {
  public:	
	virtual ValueP clone() const;
	
	String value;
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
