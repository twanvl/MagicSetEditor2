//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>

class wxSpinCtrl;

// ----------------------------------------------------------------------------- : SymbolBasicShapeEditor

/// Editor for drawing basic shapes such as rectangles and polygons
class SymbolBasicShapeEditor final : public SymbolEditorBase {
public:
  SymbolBasicShapeEditor(SymbolControl* control);
  
  // --------------------------------------------------- : Drawing
  
  void draw(DC& dc) override;
  
  // --------------------------------------------------- : UI
  
  void initUI   (wxToolBar* tb, wxMenuBar* mb) override;
  void destroyUI(wxToolBar* tb, wxMenuBar* mb) override;
  void onUpdateUI(wxUpdateUIEvent&) override;
  void onCommand(int id) override;
  int modeToolId() override;
  
  // --------------------------------------------------- : Mouse events
  
  void onLeftDown   (const Vector2D& pos, wxMouseEvent& ev) override;
  void onLeftUp     (const Vector2D& pos, wxMouseEvent& ev) override;
  void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) override;
  
  // --------------------------------------------------- : Other events
  
  void onKeyChange(wxKeyEvent& ev) override;
  
  bool isEditing() override;
  
  // --------------------------------------------------- : Data
private:
  int mode;
  SymbolShapeP shape;
  Vector2D start;
  Vector2D end;
  bool drawing;
  // controls
  wxSpinCtrl*   sides;
  wxStaticText* sidesL;
  
  /// Cancel the drawing
  void stopActions();
  
  /// Make the shape
  /**  when centered: a = center, b-a = radius
   *   otherwise:     a = top left, b = bottom right
   */
  void makeShape(Vector2D a, Vector2D b, bool constrained, bool snap, bool centered);
  
  /// Make the shape, centered in c, with radius r
  void makeCenteredShape(const Vector2D& c, Vector2D r, bool constrained);
};

