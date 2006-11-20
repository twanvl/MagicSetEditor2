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
	
	// TODO
	
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
	
	// --------------------------------------------------- : ValueViewers
	
  protected:
	/// Create an editor for the given style (as opposed to a normal viewer)
	virtual ValueViewerP makeViewer(const StyleP&);
	
	// --------------------------------------------------- : Data
  private:
	DECLARE_EVENT_TABLE();
	
	ValueViewer* current_viewer;	///< The currently selected viewer
	ValueEditor* current_editor;	///< The currently selected editor, corresponding to the viewer
	
	// --------------------------------------------------- : Events
	
	void onLeftDown  (wxMouseEvent&);
	void onLeftUp    (wxMouseEvent&);
	void onLeftDClick(wxMouseEvent&);
	void onRightDown (wxMouseEvent&);
	void onMotion    (wxMouseEvent&);
	void onMouseWheel(wxMouseEvent&);
	void onMouseLeave(wxMouseEvent&);
	
	void onChar      (wxKeyEvent&);
	
	void onContextMenu(wxContextMenuEvent&);
	void onMenu       (wxCommandEvent& e);
	
	void onFocus    (wxFocusEvent&);
	void onLoseFocus(wxFocusEvent&);
	
	// --------------------------------------------------- : Functions
	
	/// Changes the selection to the field at the specified coordinates
	/** Sends an event to the event function of the current viewer */
	void selectField(wxMouseEvent& ev, void (ValueEditor::*event)(const RealPoint&, wxMouseEvent&));
	// selectField, but don't send events
	void selectFieldNoEvents(const RealPoint& pos);
	/// Convert mouse coordinates to internal coordinates
	RealPoint mousePoint(const wxMouseEvent& e);
};

/// By default a DataEditor edits cards
typedef DataEditor CardEditor;

// ----------------------------------------------------------------------------- : EOF
#endif
