//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_EDITOR
#define HEADER_GUI_VALUE_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_editor.hpp>
#include <render/value/viewer.hpp>

// ----------------------------------------------------------------------------- : ValueEditor

/// An editor 'control' for a single value on a card
/** The inheritance diagram for derived editors looks like:
 *                 ValueViewer
 *                    ^         
 *                   /           
 *                  /             
 *         SomeValueViewer     ValueEditor
 *                 ^               ^
 *                  \             /
 *                   \           /
 *                  SomeValueEditor
 *
 *  Note that ValueEditor does NOT inherit from ValueViewer, because that leads to all kinds of problems
 */
class ValueEditor {
  public:
	// --------------------------------------------------- : Events
	
	/// This editor gains focus
	virtual void onFocus() {}
	/// This editor loses focus
	virtual void onLoseFocus() {}
	
	/// Handle mouse events
	virtual void onLeftDown   (const RealPoint& pos, wxMouseEvent& ev) {}
	virtual void onLeftUp     (const RealPoint& pos, wxMouseEvent& ev) {}
	virtual void onLeftDClick (const RealPoint& pos, wxMouseEvent& ev) {}
	virtual void onRightDown  (const RealPoint& pos, wxMouseEvent& ev) {}
	virtual void onMotion     (const RealPoint& pos, wxMouseEvent& ev) {}
	virtual void onMouseWheel (const RealPoint& pos, wxMouseEvent& ev) {}
	
	/// Key events
	virtual void onChar(wxKeyEvent ev) {}
	
	/// a context menu is requested, add extra items to the menu m
	/** return false to suppress menu */
	virtual bool onContextMenu(wxMenu& m, wxContextMenuEvent& ev) { return true; }
	/// A menu item was selected
	virtual void onMenu(wxCommandEvent& ev) { ev.Skip(); }
	
	// --------------------------------------------------- : Clipboard
	
	/// This editor can be copied from right now
	virtual bool canCopy()  const { return false; }
	/// This editor can be cut from right now
	virtual bool canCut()   const { return canCopy(); }
	/// This editor can be pasted to right now
	/** this function should also check the data on the clipboard has the right format */
	virtual bool canPaste() const { return false; }
	// Copies from this field editor, returns success
	virtual bool doCopy()   { return false; }
	// Deletes the selection from this field editor, cut = copy + delete, returns success
	virtual bool doDelete() { return false; }
	// Cuts the selection from this field editor
	bool         doCut()    { return  doCopy() && doDelete(); }
	/// Initiate pasting in this field editor,
	/** should again check if pasting is possible and fail silently if not, returns success */
	virtual bool doPaste() { return false; }
	
	// --------------------------------------------------- : Formating
	
	/// Is the given type of formatting change supported?
	virtual bool canFormat(int type) const { return false; }
	/// Is the given type of formatting enabled for the current selection?
	virtual bool hasFormat(int type) const { return false; }
	/// Toggle the given type of formatting for the current selection
	virtual void doFormat(int type)        { assert(false); }
	
	// --------------------------------------------------- : Selection
	
	/// Select the specified range (if it makes sense)
	virtual void select(size_t start, size_t end) {}
	/// Determine the selected range
	virtual size_t selectionStart() const { return 0; }
	virtual size_t selectionEnd()   const { return 0; }
	
	// --------------------------------------------------- : Other
	
	/// The cursor type to use when the mouse is over this control
	virtual wxCursor cursor() const { return wxCursor(); }
	/// determines prefered size in the native look, update the style
	virtual void determineSize() {}
	/// The editor is shown or hidden
	virtual void onShow(bool) {}
	
	/// Draw selection indicators
	/** note: the drawing of the value is done by the viewer, only a selection indicator is drawn here
	 */
	virtual void drawSelection(RotatedDC& dc) {}
};

// ----------------------------------------------------------------------------- : Utility

#define DECLARE_VALUE_EDITOR(Type)											\
		Type##ValueEditor(DataEditor& parent, const Type##StyleP& style)	\
			: Type##ValueViewer(parent, style)								\
		{}																	\
		virtual ValueEditor* getEditor() { return this; }					\
	  private:																\
		inline DataEditor& editor() const {									\
			return static_cast<DataEditor&>(viewer);						\
		}																	\
	  public:

// ----------------------------------------------------------------------------- : EOF
#endif
