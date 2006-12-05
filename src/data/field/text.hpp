//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_TEXT
#define HEADER_DATA_FIELD_TEXT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <util/rotation.hpp>
#include <data/field.hpp>
#include <data/font.hpp>
#include <data/symbol_font.hpp>
#include <script/scriptable.hpp>
#include <gfx/gfx.hpp>

// ----------------------------------------------------------------------------- : TextField

DECLARE_POINTER_TYPE(TextField);
DECLARE_POINTER_TYPE(TextStyle);
DECLARE_POINTER_TYPE(TextValue);

/// A field for values containing tagged text
class TextField : public Field {
  public:
	TextField();
	DECLARE_FIELD_TYPE(Text);
	
	OptionalScript script;			///< Script to apply to all values
	OptionalScript default_script;	///< Script that generates the default value
	bool multi_line;				///< Are newlines allowed in the text?
	bool move_cursor_with_sort;		///< When the text is reordered by a script should the cursor position be updated?
	String default_name;			///< Name of "default" value
	
	virtual void initDependencies(Context&, const Dependency&) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : TextStyle

/// The Style for a TextField
class TextStyle : public Style {
  public:
	TextStyle(const TextFieldP&);
	DECLARE_STYLE_TYPE(Text);
	
	Font font;									///< Font to use for the text
	SymbolFontRef symbol_font;					///< Symbol font for symbols in the text
	bool always_symbol;							///< Should everything be drawn as symbols?
	bool allow_formating;						///< Is formating (bold/italic/..) allowed?
	Alignment alignment;						///< Alignment inside the box
	int angle;									///< Angle of the text inside the box
	double padding_left,   padding_left_min;	///< Padding
	double padding_right,  padding_right_min;	///< Padding
	double padding_top,    padding_top_min;		///< Padding
	double padding_bottom, padding_bottom_min;	///< Padding
	double line_height_soft;					///< Line height for soft linebreaks
	double line_height_hard;					///< Line height for hard linebreaks
	double line_height_line;					///< Line height for <line> tags
	String mask_filename;						///< Filename of the mask
	ContourMask mask;							///< Mask to fit the text to (may be null)	
	
	virtual bool update(Context&);
	virtual void initDependencies(Context&, const Dependency&) const;
	
	/// The rotation to use when drawing
	inline Rotation getRotation() const {
		return Rotation(angle, getRect());
	}
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : TextValue

/// The Value in a TextField
class TextValue : public Value {
  public:
	inline TextValue(const TextFieldP& field) : Value(field) {}
	DECLARE_HAS_FIELD(Text)
	
	typedef Defaultable<String> ValueType;
	ValueType value;				///< The text of this value
	
	virtual String toString() const;
	virtual bool update(Context&);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
