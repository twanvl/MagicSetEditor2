//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <render/text/viewer.hpp>
#include <data/field/text.hpp>

// ----------------------------------------------------------------------------- : TextValueViewer

/// Viewer that displays a text value
class TextValueViewer : public ValueViewer {
  public:
  DECLARE_VALUE_VIEWER(Text) : ValueViewer(parent,style) {}
  
  virtual bool prepare(RotatedDC& dc);
  virtual void draw(RotatedDC& dc);
  virtual void onValueChange();
  virtual void onStyleChange(int);
  virtual void onAction(const Action&, bool undone);
  virtual double getStretch() const;
  
  protected:
  TextViewer v;
};


