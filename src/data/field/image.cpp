//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/image.hpp>

// ----------------------------------------------------------------------------- : ImageField

FIELD_TYPE(Image)

String ImageField::typeName() const {
	return _("image");
}

IMPLEMENT_REFLECTION(ImageField) {
	REFLECT_BASE(Field);
}


// ----------------------------------------------------------------------------- : ImageStyle

IMPLEMENT_REFLECTION(ImageStyle) {
	REFLECT_BASE(Style);
	REFLECT_N("mask", mask_filename);
}


// ----------------------------------------------------------------------------- : ImageValue

String ImageValue::toString() const {
	return filename.empty() ? wxEmptyString : _("<image>");
}

IMPLEMENT_REFLECTION_NAMELESS(ImageValue) {
	REFLECT_NAMELESS(filename);
}
