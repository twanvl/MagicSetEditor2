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
	virtual ~ValueEditor() {}
	// --------------------------------------------------- : Events
	
	/// This editor gains focus
	virtual void onFocus() {}
	/// This editor loses focus
	virtual void onLoseFocus() {}
	
	/// Handle mouse events, return true if the event is used
	virtual bool onLeftDown   (const RealPoint& pos, wxMouseEvent& ev) { return false; }
	virtual bool onLeftUp     (const RealPoint& pos, wxMouseEvent& ev) { return false; }
	virtual bool onLeftDClick (const RealPoint& pos, wxMouseEvent& ev) { return false; }
	virtual bool onRightDown  (const RealPoint& pos, wxMouseEvent& ev) { return false; }
	virtual bool onMotion     (const RealPoint& pos, wxMouseEvent& ev) { return false; }
	virtual bool onMouseWheel (const RealPoint& pos, wxMouseEvent& ev) { return false; }
	
	/// Key events
	virtual bool onChar(wxKeyEvent& ev) { return false; }
	
	/// a context menu is requested, add extra items to the menu m
	/** return false to suppress menu */
	virtual bool onContextMenu(wxMenu& m, wxContextMenuEvent& ev) { return true; }
	/// Get a special menu, events will be sent to onMenu
	virtual wxMenu* getMenu(int type) const { return nullptr; }
	/// A menu item was selected, return true if the command was processed
	virtual bool onCommand(int id) { return false; }
	
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
	bool         doCut()    { return doCopy() && doDelete(); }
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
	
	// --------------------------------------------------- : Search / replace
	
	/// Do a search or replace action for the given FindInfo
	/** If from_start == false: searches only from the current selection onward (or backward),
	 *                          excluding the sellection itself.
	 *  If from_start == true:  searches everything
	 *
	 *  Returns true when more searching is needed.
	 */
	bool search(FindInfo& find, bool from_start);
	
	// --------------------------------------------------- : Other
	
	/// The cursor type to use when the mouse is over this control
	virtual wxCursor cursor() const { return wxCursor(); }
	/// Determines prefered size in the native look, update the style
	virtual void determineSize(bool force_fit = false) {}
	/// Should a label and control border be drawn in the native look?
	virtual bool drawLabel() const { return true; }
	/// The editor is shown or hidden
	virtual void onShow(bool) {}
	
	/// Redraw this viewer
	virtual void redraw() = 0;
};

// ----------------------------------------------------------------------------- : Utility

#define DECLARE_VALUE_EDITOR(Type)											\
		Type##ValueEditor(DataEditor& parent, const Type##StyleP& style);	\
		virtual ValueEditor* getEditor() { return this; }					\
		virtual void redraw();												\
	  private:																\
		inline DataEditor& editor() const {									\
			return static_cast<DataEditor&>(viewer);						\
		}																	\
	  public:

#define IMPLEMENT_VALUE_EDITOR(Type)													\
	void Type##ValueEditor::redraw() {													\
		editor().redraw(*this);															\
	}																					\
	Type##ValueEditor::Type##ValueEditor(DataEditor& parent, const Type##StyleP& style)	\
		: Type##ValueViewer(parent, style)

// ----------------------------------------------------------------------------- : EOF
#endif
