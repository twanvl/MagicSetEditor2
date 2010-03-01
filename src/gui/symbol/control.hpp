//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_CONTROL
#define HEADER_GUI_SYMBOL_CONTROL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol.hpp>
#include <gui/symbol/selection.hpp>
#include <render/symbol/viewer.hpp>

class SymbolWindow;
DECLARE_POINTER_TYPE(SymbolEditorBase);

// ----------------------------------------------------------------------------- : SymbolControl

/// Control for editing symbols
/** What kind of editing is done is determined by the contained SymbolEditorBase object
 *  That object handles all events and the drawing. This class is mostly just a proxy.
 */
class SymbolControl : public wxControl, public SymbolViewer {
  public:
	SymbolControl(SymbolWindow* parent, int id, const SymbolP& symbol);
	
	virtual void onChangeSymbol();
	
	virtual void onAction(const Action&, bool undone);
	
	// Forward command to editor
	void onExtraTool(wxCommandEvent& ev);
	
	// Switch to some editing mode
	void onModeChange(wxCommandEvent& ev);
	
	/// Handle UpdateUIEvents propagated from the SymbolWindow
	/** Handles events for editing mode related stuff
	 */
	void onUpdateUI(wxUpdateUIEvent& ev);
	
	/// The selection has changed, tell the part list
	void signalSelectionChange();
	
	/// Activate a part, open it in the point editor, if it is a shape
	void activatePart(const SymbolPartP& part);
	
	/// Select a specific part from the symbol
	/// The editor is switched to the select editor
	void selectPart(const SymbolPartP& part);
	
	/// Update the selection
	void onUpdateSelection();
	
	/// Are we editing?
	bool isEditing();
	
  private:
	/// Switch the a different editor object
	void switchEditor(const SymbolEditorBaseP& e);
		
	/// Draw the editor
	void draw(DC& dc);
	
  private:
	DECLARE_EVENT_TABLE();

	// --------------------------------------------------- : Data
	
  public: 
	/// What parts are selected?
	SymbolPartsSelection selected_parts;
	SymbolPartP          highlight_part;    ///< part the mouse cursor is over
	SymbolShapeP         selected_shape;    ///< if there is a single selection
	SymbolSymmetryP      selected_symmetry; ///< if there is a single selection
	
	/// Parent window 
	SymbolWindow* parent;
	
  private:
	/// The current editor
	SymbolEditorBaseP editor;
	
	/// Last mouse position
	Vector2D last_pos;
	
	// --------------------------------------------------- : Events
	
	void onLeftDown  (wxMouseEvent& ev);
	void onLeftUp    (wxMouseEvent& ev);
	void onLeftDClick(wxMouseEvent& ev);
	void onRightDown (wxMouseEvent& ev);
	void onMotion    (wxMouseEvent& ev);
		
	void onPaint    (wxPaintEvent& ev);
	void onKeyChange(wxKeyEvent& ev);
	void onChar     (wxKeyEvent& ev);
	void onSize     (wxSizeEvent& ev);
};


// ----------------------------------------------------------------------------- : EOF
#endif
