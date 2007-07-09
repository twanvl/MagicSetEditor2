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

// ----------------------------------------------------------------------------- : SymbolSymmetryEditor

/// Editor for adding symmetries
class SymbolSymmetryEditor : public SymbolEditorBase {
  public:
	SymbolSymmetryEditor(SymbolControl* control);
	
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
	virtual void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	
	// --------------------------------------------------- : Other events
	
	virtual void onKeyChange(wxKeyEvent& ev);
	
	virtual bool isEditing();
	
	// --------------------------------------------------- : Data
  private:
	int mode;
	SymbolSymmetryP symmetry;
	bool drawing;
	Vector2D center, handle;
	// controls
	wxSpinCtrl* copies;
	
	/// Cancel the drawing
	void stopActions();
	
	/// Make the symmetry object
	void makePart(Vector2D a, Vector2D b, bool constrained, bool snap);
	
};

// ----------------------------------------------------------------------------- : EOF
#endif
