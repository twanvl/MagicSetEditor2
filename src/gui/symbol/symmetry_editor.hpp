//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_SYMMETRY_EDITOR
#define HEADER_GUI_SYMBOL_SYMMETRY_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>
#include <wx/spinctrl.h>

class SymmetryMoveAction;

// ----------------------------------------------------------------------------- : SymbolSymmetryEditor

/// Editor for adding symmetries
class SymbolSymmetryEditor : public SymbolEditorBase {
  public:
	/** The symmetry parameter is optional, if it is not set, then only new ones can be created */
	SymbolSymmetryEditor(SymbolControl* control, const SymbolSymmetryP& symmetry);
	
	// --------------------------------------------------- : Drawing
	
	virtual void draw(DC& dc);
	
	// --------------------------------------------------- : UI
	
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	virtual int modeToolId();
	
	// --------------------------------------------------- : Mouse events
	
	virtual void onLeftDown   (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onLeftUp     (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	virtual void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	
	// --------------------------------------------------- : Other events
	
	virtual void onKeyChange(wxKeyEvent& ev);
	
	virtual bool isEditing();
	
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
		SELECTION_HANDLE,	// dragging a handle
		SELECTION_CENTER,	// dragging the rotation center
	} selection, hovered;
	
	Selection findSelection(const Vector2D& pos);
	
	/// Done with dragging
	void resetActions();
	
};

// ----------------------------------------------------------------------------- : EOF
#endif
