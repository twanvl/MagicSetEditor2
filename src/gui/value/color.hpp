//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/color.hpp>

DECLARE_SHARED_POINTER_TYPE(DropDownList);

// ----------------------------------------------------------------------------- : ColorValueEditor

/// An editor 'control' for editing ColorValues
class ColorValueEditor : public ColorValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Color);
  
  // --------------------------------------------------- : Events
  bool onLeftDown(const RealPoint& pos, wxMouseEvent& ev) override;
  bool onChar(wxKeyEvent& ev) override;
  void onLoseFocus() override;
  
  void draw(RotatedDC& dc) override;
  void determineSize(bool) override;
  
private:
  DropDownListP drop_down;
  friend class DropDownColorList;
  /// Change the color
  void change(const Defaultable<Color>& c);
  /// Change to a custom color
  void changeCustom();
};

