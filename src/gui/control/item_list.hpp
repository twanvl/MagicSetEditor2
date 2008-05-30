//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_ITEM_LIST
#define HEADER_GUI_CONTROL_ITEM_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/listctrl.h>

// ----------------------------------------------------------------------------- : ItemList

/// A generic list of items
/** The list is shown in report mode, and allows sorting.
 *  Currently used for cards and keywords.
 *
 *  Terminology:
 *     selected item = a single item in the variable selected_item
 *     focused items = items that are drawn as selected in the control
 */
class ItemList : public wxListView {
  public:
	ItemList(Window* parent, int id, long additional_style = 0, bool multi_sel = false);
	
	// --------------------------------------------------- : Selection
	
	/// Is there a previous item to select?
	bool canSelectPrevious() const;
	/// Is there a next item to select?
	bool canSelectNext() const;
	/// Move the selection to the previous item (if possible)
	void selectPrevious();
	/// Move the selection to the next item (if possible)
	void selectNext();
	/// Move the selection to the first item (if possible)
	void selectFirst();
	
	// --------------------------------------------------- : Clipboard
	
	virtual bool canCut()    const { return canCopy() && canDelete(); }
	virtual bool canCopy()   const { return false; }
	virtual bool canPaste()  const { return false; }
	virtual bool canDelete() const { return false; }
	// Try to perform a clipboard operation, return success
	virtual bool doCut();
	virtual bool doCopy()   { return false; }
	virtual bool doPaste()  { return false; }
	virtual bool doDelete() { return false; }
	
	// --------------------------------------------------- : Virtual interface
  protected:
	/// Get a list of all items
	virtual void getItems(vector<VoidP>& out) const = 0;
	
	/// Send an 'item selected' event for the currently selected item (selected_item)
	virtual void sendEvent() = 0;
	
	/// Is sorting required?
	virtual bool mustSort() const { return false; }
	/// Compare two items for < based on sort_by_column (not on sort_ascending)
	virtual bool compareItems(void* a, void* b) const = 0;
	
	// --------------------------------------------------- : Protected interface
	/// Return the card at the given position in the sorted list
	inline const VoidP& getItem(long pos) const { return sorted_list[pos]; }
	/// Sort by the given column
	virtual void sortBy(long column, bool ascending);
	/// Refresh the card list (resort, refresh and reselect current item)
	void refreshList();
	/// Set the image of a column header (fixes wx bug)
	void SetColumnImage(int col, int image);
	
	/// Select an item, send an event to the parent
	/** If focus then the item is also focused and selected in the actual control.
	 *  This should not be done when the item is selected because it was focused (leading to a loop).
	 */
	void selectItem(const VoidP& item, bool focus, bool event);
	/// Select a item at the specified position
	void selectItemPos(long pos, bool focus, bool force_focus = false);
	/// Find the position for the selected_item
	void findSelectedItemPos();
	/// Actually select the item at selected_item_pos in the control
	/** if force_focus == true, then the item is highlighted again if it already is focused. */
	void focusSelectedItem(bool force_focus = false);
	/// Deselect everything in the control
	void focusNone();
	/// Actually select a certain item in the control
	void focusItem(const VoidP& item, bool focus = true);
	/// Count the number of focused items
	long focusCount() const;
	
	// --------------------------------------------------- : Data
	VoidP          selected_item;	  ///< The currently selected item
	long           selected_item_pos; ///< Position of the selected item in the sorted_list, or -1 if no card is selected
	long           sort_by_column;    ///< Column to use for sorting, or -1 if not sorted
	bool           sort_ascending;    ///< Sort order
	vector<VoidP>  sorted_list;       ///< Sorted list of items, can be considered a map: pos->item
	
  private:
	struct ItemComparer; // for comparing items
	
	// --------------------------------------------------- : Window events
	DECLARE_EVENT_TABLE();
	
	void onColumnClick(wxListEvent& ev);
	void onItemFocus  (wxListEvent& ev);
	void onContextMenu(wxContextMenuEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
