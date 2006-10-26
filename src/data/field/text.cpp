//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/text.hpp>
#include <script/script.hpp>

// ----------------------------------------------------------------------------- : TextField

TextField::TextField()
	: multi_line(false), move_cursor_with_sort(false)
	, default_name(_("Default"))
{}

IMPLEMENT_FIELD_TYPE(Text)

String TextField::typeName() const {
	return _("text");
}


IMPLEMENT_REFLECTION(TextField) {
	REFLECT_BASE(Field);
	REFLECT(multi_line);
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(move_cursor_with_sort);
	REFLECT(default_name);
}

// ----------------------------------------------------------------------------- : TextStyle

TextStyle::TextStyle(const TextFieldP& field)
	: Style(field)
	, always_symbol(false), allow_formating(true)
	, alignment(ALIGN_TOP_LEFT)
	, angle(0)
	, padding_left  (0), padding_left_min  (10000)
	, padding_right (0), padding_right_min (10000)
	, padding_top   (0), padding_top_min   (10000)
	, padding_bottom(0), padding_bottom_min(10000)
	, line_height_soft(1.0)
	, line_height_hard(1.0)
	, line_height_line(1.0)
{}

IMPLEMENT_REFLECTION(TextStyle) {
	REFLECT_BASE(Style);
//	REFLECT(font);
//	REFLECT(symbol_font);
	REFLECT(always_symbol);
	REFLECT(allow_formating);
	REFLECT(alignment);
	REFLECT(angle);
	REFLECT(padding_left);
	REFLECT(padding_right);
	REFLECT(padding_top);
	REFLECT(padding_bottom);
	REFLECT(padding_left_min);
	REFLECT(padding_right_min);
	REFLECT(padding_top_min);
	REFLECT(padding_bottom_min);
	REFLECT(line_height_soft);
	REFLECT(line_height_hard);
	REFLECT(line_height_line);
	REFLECT_N("mask", mask_filename);
}

// ----------------------------------------------------------------------------- : TextValue

String TextValue::toString() const {
	return value();
}

IMPLEMENT_REFLECTION_NAMELESS(TextValue) {
	REFLECT_NAMELESS(value);
}
