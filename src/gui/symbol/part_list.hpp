//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_PART_LIST
#define HEADER_GUI_SYMBOL_PART_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol.hpp>
#include <wx/listctrl.h>

// ----------------------------------------------------------------------------- : SymbolPartList

// A list view of parts of a symbol
class SymbolPartList : public wxListCtrl, public SymbolView {
  public:
	SymbolPartList(Window* parent, int id, SymbolP symbol = SymbolP());
	
	/// Update the list
	void update();
	
	/// Is there a selection?
	inline bool hasSelection() const { return selected != -1; }
	/// Return the last part that was selected
	/** @pre hasSelection()
	 */
	inline SymbolPartP getSelection() const { return getPart(selected); }
	
	/// Get a set of selected parts
	void getSelectedParts(set<SymbolPartP>& sel);
	/// Select the specified parts, and nothing else
	void selectParts(const set<SymbolPartP>& sel);
	
	/// Another symbol is being viewed
	void onChangeSymbol();
	
	/// Event handler for changes to the symbol
	virtual void onAction(const Action& a);
	
  protected:
	/// Get the text of an item
	virtual String OnGetItemText(long item, long col) const;
	/// Get the icon of an item
	virtual int OnGetItemImage(long item) const;
	
  private:
	/// The selected item, or -1 if there is no selection
	long selected;
	
	/// Get a part from the symbol
	SymbolPartP getPart(long item) const;
	
	/// Select an item, also in the list control
	/// Deselects all other items
	void selectItem(long item);
	
	// --------------------------------------------------- : Event handling
	DECLARE_EVENT_TABLE();
	
	void onSelect   (wxListEvent& ev);
	void onDeselect (wxListEvent& ev);
	void onLabelEdit(wxListEvent& ev);
	void onSize     (wxSizeEvent& ev);
	void onDrag     (wxMouseEvent& ev);
};

// ----------------------------------------------------------------------------- : EOF
#endif
