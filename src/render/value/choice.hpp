//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/choice.hpp>

// ----------------------------------------------------------------------------- : ChoiceValueViewer

/// Viewer that displays a choice value
class ChoiceValueViewer : public ValueViewer {
public:
  DECLARE_VALUE_VIEWER(Choice) : ValueViewer(parent,style) {}
  
  bool prepare(RotatedDC& dc) override;
  void draw(RotatedDC& dc) override;
  void onStyleChange(int) override;
};

bool prepare_choice_viewer(RotatedDC& dc, ValueViewer& viewer, ChoiceStyle& style, const String& value);
void draw_choice_viewer(RotatedDC& dc, ValueViewer& viewer, ChoiceStyle& style, const String& value);

