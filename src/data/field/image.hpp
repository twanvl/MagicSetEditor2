//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_IMAGE
#define HEADER_DATA_FIELD_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>

// ----------------------------------------------------------------------------- : ImageField

/// A field for image values
class ImageField : public Field {
  public:
	// no extra data
	
	virtual ValueP newValue(const FieldP& thisP) const;
	virtual StyleP newStyle(const FieldP& thisP) const;
	virtual String typeName() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ImageStyle

/// The Style for a ImageField
class ImageStyle : public Style {
  public:
	Scriptable<String> mask_filename; ///< Filename for a mask image
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ImageValue

/// The Value in a ImageField, i.e. an image
class ImageValue : public Value {
  public:
	String filename; ///< Filename of the image (in the current package), or ""
	
	virtual String toString() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
