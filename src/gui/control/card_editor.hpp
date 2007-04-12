//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_CARD_EDITOR
#define HEADER_GUI_CONTROL_CARD_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_viewer.hpp>

class ValueEditor;
class FindInfo;

// ----------------------------------------------------------------------------- : DataEditor

/// An editor for data values (usually a card)
class DataEditor : public CardViewer {
  public:
	DataEditor(Window* parent, int id, long style = 0);
	
	// --------------------------------------------------- : Utility for ValueViewers
	
	virtual bool drawBorders() const;
	virtual bool drawEditing() const;
	virtual wxPen borderPen(bool active) const;
	virtual ValueViewer* focusedViewer() const;
	
	// --------------------------------------------------- : Selection
	
	/// Select the given viewer, sends focus events
	void select(ValueViewer* v);
	/// Select the first editable and visible editor (by tab index)
	void selectFirst();
	/// Select the next editable editor, returns false if the current editor is the last one
	bool selectNext();
	/// Select the previous editable editor, returns false if the current editor is the first one
	bool selectPrevious();
	
	// --------------------------------------------------- : Clipboard
	
	bool canCut()   const;
	bool canCopy()  const;
	bool canPaste() const;
	void doCut();
	void doCopy();
	void doPaste();
	
	// --------------------------------------------------- : Formatting
	
	bool canFormat(int type) const;
	bool hasFormat(int type) const;
	void doFormat (int type);
	/// Get a special menu, events should be sent to onCommand
	wxMenu* getMenu(int type) const;
	/// A menu item from getMenu was selected
	void onCommand(int id);
	
	// --------------------------------------------------- : Search/replace
		
	/// Do a search or replace action for the given FindInfo
	/** If from_start == false: searches only from the current selection onward (or backward)
	 *  If from_start == true:  searches everything
	 *
	 *  Returns true when more searching is needed.
	 */
	bool search(FindInfo& find, bool from_start);
	
	// --------------------------------------------------- : ValueViewers
	
  protected:
	/// Create an editor for the given style (as opposed to a normal viewer)
	virtual ValueViewerP makeViewer(const StyleP&);
	
	virtual void onInit();
	
	// --------------------------------------------------- : Data
	ValueViewer* current_viewer;	///< The currently selected viewer
	ValueEditor* current_editor;	///< The currently selected editor, corresponding to the viewer
	vector<ValueViewer*> by_tab_index;	///< The editable viewers, sorted by tab index
	
  private:
	// --------------------------------------------------- : Events
	DECLARE_EVENT_TABLE();
	
	
	void onLeftDown  (wxMouseEvent&);
	void onLeftUp    (wxMouseEvent&);
	void onLeftDClick(wxMouseEvent&);
	void onRightDown (wxMouseEvent&);
	void onMotion    (wxMouseEvent&);
	void onMouseWheel(wxMouseEvent&);
	void onMouseLeave(wxMouseEvent&);
	
	void onChar      (wxKeyEvent&);
	
	void onContextMenu(wxContextMenuEvent&);
	void onMenu       (wxCommandEvent&);
	
	void onFocus    (wxFocusEvent&);
	void onLoseFocus(wxFocusEvent&);
	
	// --------------------------------------------------- : Functions
	
	/// Changes the selection to the field at the specified coordinates
	/** Sends an event to the event function of the current viewer */
	void selectField(wxMouseEvent& ev, bool (ValueEditor::*event)(const RealPoint&, wxMouseEvent&));
	// selectField, but don't send events
	void selectFieldNoEvents(const RealPoint& pos);
	/// Convert mouse coordinates to internal coordinates
	RealPoint mousePoint(const wxMouseEvent&);
	
	// Create tab index ordering of the (editable) viewers
	void createTabIndex();
	/// Select the field with the given position in the by_tab_index list
	/** Returns success */
	bool selectByTabPos(int tab_pos);
	/// Find the tab pos of the current viewer, returns -1 if not found
	int currentTabPos() const;
};

/// By default a DataEditor edits cards
typedef DataEditor CardEditor;

// ----------------------------------------------------------------------------- : Utility

#define FOR_EACH_EDITOR							\
	FOR_EACH(v, viewers)						\
		if (ValueEditor* e = v->getEditor())
#define FOR_EACH_EDITOR_REVERSE					\
	FOR_EACH_REVERSE(v, viewers)				\
		if (ValueEditor* e = v->getEditor())

// ----------------------------------------------------------------------------- : EOF
#endif
