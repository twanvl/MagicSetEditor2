//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/multiple_choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceValueViewer

/// Viewer that displays a multiple choice value
class MultipleChoiceValueViewer : public ValueViewer {
  public:
  DECLARE_VALUE_VIEWER(MultipleChoice) : ValueViewer(parent,style), item_height(0) {}
  
  virtual bool prepare(RotatedDC& dc);
  virtual void draw(RotatedDC& dc);
  virtual void onStyleChange(int);
  protected:
  double item_height; ///< Height of a single item, or 0 if non uniform
  private:
  void drawChoice(RotatedDC& dc, RealPoint& pos, const String& choice, bool active = true);
};

