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
class SymmetryMoveAction;

// ----------------------------------------------------------------------------- : SymbolSymmetryEditor

/// Editor for adding symmetries
class SymbolSymmetryEditor final : public SymbolEditorBase {
public:
  /** The symmetry parameter is optional, if it is not set, then only new ones can be created */
  SymbolSymmetryEditor(SymbolControl* control, const SymbolSymmetryP& symmetry);
  
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
  void onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) override;
  void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) override;
  
  // --------------------------------------------------- : Other events
  
  void onKeyChange(wxKeyEvent& ev) override;
  
  bool isEditing() override;
  
  // --------------------------------------------------- : Data
private:
  SymbolSymmetryP& symmetry;
  // controls
  wxSpinCtrl* copies;
  // Actions
  SymmetryMoveAction* symmetryMoveAction;
  
  // What is selected?
  enum Selection {
    SELECTION_NONE,
    SELECTION_HANDLE,  // dragging a handle
    SELECTION_CENTER,  // dragging the rotation center
  } selection, hovered;
  
  Selection findSelection(const Vector2D& pos);
  
  /// Done with dragging
  void resetActions();
  
};

