//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_IMAGE
#define HEADER_DATA_FIELD_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>

// ----------------------------------------------------------------------------- : ImageField

DECLARE_POINTER_TYPE(ImageField);
DECLARE_POINTER_TYPE(ImageStyle);
DECLARE_POINTER_TYPE(ImageValue);

/// A field for image values
class ImageField : public Field {
  public:
	// no extra data
	DECLARE_FIELD_TYPE(Image);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ImageStyle

/// The Style for a ImageField
class ImageStyle : public Style {
  public:
	inline ImageStyle(const ImageFieldP& field) : Style(field) {}
	DECLARE_STYLE_TYPE(Image);
	
	Scriptable<String> mask_filename; ///< Filename for a mask image
	
	virtual bool update(Context&);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ImageValue

/// The Value in a ImageField, i.e. an image
class ImageValue : public Value {
  public:
	inline ImageValue(const ImageFieldP& field) : Value(field) {}
	
	typedef FileName ValueType;
	ValueType filename; ///< Filename of the image (in the current package), or ""
	
	virtual String toString() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
