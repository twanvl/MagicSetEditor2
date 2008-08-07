//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/image.hpp>
#include <gfx/generated_image.hpp>

// ----------------------------------------------------------------------------- : ImageField

IMPLEMENT_FIELD_TYPE(Image, "image");

IMPLEMENT_REFLECTION(ImageField) {
	REFLECT_BASE(Field);
}


// ----------------------------------------------------------------------------- : ImageStyle

IMPLEMENT_REFLECTION(ImageStyle) {
	REFLECT_BASE(Style);
	REFLECT_N("mask", mask_filename);
	REFLECT_N("default", default_image);
}

int ImageStyle::update(Context& ctx) {
	return Style       ::update(ctx)
	     | mask_filename.update(ctx) * CHANGE_MASK
	     | default_image.update(ctx) * CHANGE_DEFAULT;
}

// ----------------------------------------------------------------------------- : ImageValue

String ImageValue::toString() const {
	return filename.empty() ? wxEmptyString : _("<image>");
}

// custom reflection: convert to ScriptImageP for scripting

void ImageValue::reflect(Reader& tag) {
	tag.handle(filename);
}
void ImageValue::reflect(Writer& tag) {
	if (fieldP->save_value) tag.handle(filename);
}
void ImageValue::reflect(GetMember& tag) {}
void ImageValue::reflect(GetDefaultMember& tag) {
	// convert to ScriptImageP for scripting
	tag.handle( (ScriptValueP)new_intrusive2<ImageValueToImage>(filename, last_update) );
}
