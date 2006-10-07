//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_EDITOR
#define HEADER_GUI_SYMBOL_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol.hpp>
#include <gui/symbol/control.hpp>

class SymbolControl;

// ----------------------------------------------------------------------------- : SymbolEditorBase

/// Base class for editors of symbols.
/** A symbol editor is like a FieldEditor, events are forwarded to it.
 *  Differrent SymbolEditors represent different tools.
 *  NOTE : Do not confuse with SymbolEditor (a FieldEditor)
 */
class SymbolEditorBase {
  protected:
	/// The control for which we are editing
	SymbolControl& control;
	
	inline SymbolP getSymbol() { return control.getSymbol(); }
	void SetStatusText(const String& text);
	
  public:
	SymbolEditorBase(SymbolControl* control)
		: control(*control)
	{}
	virtual ~SymbolEditorBase() {};
	
	// --------------------------------------------------- : Drawing
	
	/// Drawing for this control,
	virtual void draw(DC& dc) = 0;
	
	// --------------------------------------------------- : UI
	
	/// Init extra toolbar items and menus needed for this editor
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb) {}
	/// Destroy the extra items added by initUI.
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb) {}
	/// Update the UI by enabling/disabling items
	virtual void onUpdateUI(wxUpdateUIEvent& ev) {}
	/// Respond to one of the extra menu/tool items
	virtual void onCommand(int id) {}
	/// Tool id used in the symbol window
	virtual int modeToolId() = 0;
	
	// --------------------------------------------------- : Mouse events
	
	/// The left mouse button has been pressed, at the given position (internal coordinates)
	virtual void onLeftDown   (const Vector2D& pos, wxMouseEvent& ev) {}
	/// The left mouse button has been released, at the given position (internal coordinates)
	virtual void onLeftUp     (const Vector2D& pos, wxMouseEvent& ev) {}
	/// The left mouse button has been double clicked, at the given position (internal coordinates)
	virtual void onLeftDClick (const Vector2D& pos, wxMouseEvent& ev) {}
	/// The right mouse button has been pressed, at the given position (internal coordinates)
	virtual void onRightDown  (const Vector2D& pos, wxMouseEvent& ev) {}
	/// The mouse is being moved, no mouse buttons are pressed
	virtual void onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {}
	/// The mouse is being moved while mouse buttons are being pressed
	virtual void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {}
	
	// --------------------------------------------------- : Keyboard events
	
	/// A key is pressed or released, should be used for modifier keys (Shift/Ctrl/Alt)
	virtual void onKeyChange  (wxKeyEvent& ev) {}
	/// A key is pressed/clicked
	virtual void onChar       (wxKeyEvent& ev) {}
	
	// --------------------------------------------------- : Other events
	/// A context menu is requested
	virtual void onContextMenu(wxContextMenuEvent& ev) {}
	
	/// Is the user currently editing, i.e. dragging the mouse?
	/** This disables undo/redo, so the current action is not
	 *  undone while it is in progress.
	 */
	virtual bool isEditing() { return false; }
};

// ----------------------------------------------------------------------------- : EOF
#endif
