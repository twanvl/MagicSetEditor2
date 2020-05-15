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
  
  bool prepare(RotatedDC& dc) override;
  void draw(RotatedDC& dc) override;
  void onValueChange() override;
  void onStyleChange(int) override;
  void onAction(const Action&, bool undone) override;
  double getStretch() const override;
  
protected:
  TextViewer v;
};


