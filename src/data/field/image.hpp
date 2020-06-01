//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>
#include <script/image.hpp>
#include <util/io/package.hpp>

// ----------------------------------------------------------------------------- : ImageField

DECLARE_POINTER_TYPE(ImageField);
DECLARE_POINTER_TYPE(ImageStyle);
DECLARE_POINTER_TYPE(ImageValue);

/// A field for image values
class ImageField : public Field {
public:
  ImageField() {
    show_statistics = false; // no statistics on image fields by default, that just leads to errors
  }
  // no extra data
  DECLARE_FIELD_TYPE(Image);
};

// ----------------------------------------------------------------------------- : ImageStyle

/// The Style for a ImageField
class ImageStyle : public Style {
public:
  inline ImageStyle(const ImageFieldP& field) : Style(field) {}
  DECLARE_STYLE_TYPE(Image);
  
  ScriptableImage default_image; ///< Placeholder
  
  int update(Context&) override;
};

// ----------------------------------------------------------------------------- : ImageValue

/// The Value in a ImageField, i.e. an image
class ImageValue : public Value {
public:
  inline ImageValue(const ImageFieldP& field) : Value(field) {}
  DECLARE_VALUE_TYPE(Image, LocalFileName);
  
  ValueType filename;    ///< Filename of the image (in the current package), or ""
  Age       last_update; ///< When was the image last changed?
};

