//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/symbol.hpp>

class ValueActionPerformer;

// ----------------------------------------------------------------------------- : SymbolValueEditor

/// An editor 'control' for editing SymbolValues
class SymbolValueEditor : public SymbolValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Symbol);
  
  void draw(RotatedDC& dc) override;
  bool onLeftDown  (const RealPoint& pos, wxMouseEvent&) override;
  bool onLeftUp    (const RealPoint& pos, wxMouseEvent&) override;
  bool onLeftDClick(const RealPoint& pos, wxMouseEvent&) override;
  bool onMotion    (const RealPoint& pos, wxMouseEvent&) override;
  void determineSize(bool) override;
private:
  /// Draw a button, buttons are numbered from the right
  void drawButton(RotatedDC& dc, int button, const String& text);
  /// Is there a button at the given position? returns the button index, or -1 if there is no button
  int findButton(const RealPoint& pos);
  /// Show the symbol editor
  void editSymbol();
  /// Get an object to perform actions for us
  unique_ptr<ValueActionPerformer> getActionPerformer();
  
  // button, or -1 for mouse down, but not on button, or -2 for mouse not down
  int button_down;
  Bitmap button_images[1];
};

