//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/item_list.hpp>
#include <gui/util.hpp>
#include <wx/imaglist.h>

// ----------------------------------------------------------------------------- : ItemList

ItemList::ItemList(Window* parent, int id, long additional_style, bool multi_sel)
	: wxListView(parent, id, wxDefaultPosition, wxDefaultSize, additional_style | wxLC_REPORT | wxLC_VIRTUAL | (multi_sel ? 0 : wxLC_SINGLE_SEL))
	, selected_item_pos(-1)
	, sort_by_column(-1), sort_ascending(true)
{
	// create image list
	wxImageList* il = new wxImageList(18,14);
	il->Add(load_resource_image(_("sort_asc")),  Color(255,0,255));
	il->Add(load_resource_image(_("sort_desc")), Color(255,0,255));
	AssignImageList(il, wxIMAGE_LIST_SMALL);
}

// ----------------------------------------------------------------------------- : ItemList : Selection

bool ItemList::canSelectPrevious() const {
	return selected_item_pos - 1 >= 0;
}
bool ItemList::canSelectNext() const {
	return selected_item_pos >= 0 && static_cast<size_t>(selected_item_pos + 1) < sorted_list.size();
}
void ItemList::selectPrevious() {
	assert(selected_item_pos >= 1);
	focusNone();
	selectItemPos(selected_item_pos - 1, true, true);
}
void ItemList::selectNext() {
	assert(selected_item_pos + 1 < (long)sorted_list.size());
	focusNone();
	selectItemPos(selected_item_pos + 1, true, true);
}
void ItemList::selectFirst() {
	if (sorted_list.empty()) return;
	selectItemPos(0, true);
}

bool ItemList::doCut() {
	// cut = copy + delete
	if (!canCut()) return false;
	if (!doCopy()) return false;
	doDelete();
	return true;
}

// ----------------------------------------------------------------------------- : ItemList : Selection (private)

void ItemList::selectItem(const VoidP& item, bool focus, bool event) {
	if (item != selected_item && focus) {
		focusNone();
	}
	selected_item = item;
	if (event) sendEvent();
	findSelectedItemPos();
	if (focus) focusSelectedItem();
}

void ItemList::selectItemPos(long pos, bool focus, bool force_focus) {
	VoidP item;
	if ((size_t)pos < sorted_list.size()) {
		item = getItem(pos);
	} else if (!sorted_list.empty()) {
		item = sorted_list.back();
	} else {
		// clear selection
	}
	if (item != selected_item) {
		selectItem(item, false, true);
	}
	//!selected_item_pos = pos;
	if (focus) focusSelectedItem(force_focus);
}

void ItemList::findSelectedItemPos() {
	// find the position of the selected item
	long count = GetItemCount();
	selected_item_pos = -1;
	for (long pos = 0 ; pos < count ; ++pos) {
		if (getItem(pos) == selected_item) {
			selected_item_pos = pos;
			break;
		}
	}
}
void ItemList::focusSelectedItem(bool force_focus) {
	if (GetItemCount() > 0) {
		if (selected_item_pos == -1 || (size_t)selected_item_pos > sorted_list.size()) {
			// deselect currently selected item, if any
			long sel = GetFirstSelected();
			Select(sel, false);
		} else if (selected_item_pos != GetFocusedItem() || force_focus) {
			Select(selected_item_pos);
			Focus (selected_item_pos);
		}
	}
}
void ItemList::focusNone() {
	long count = GetItemCount();
	for (long pos = 0 ; pos < count ; ++pos) {
		Select(pos, false);
	}
}
void ItemList::focusItem(const VoidP& item, bool focus) {
	long count = GetItemCount();
	for (long pos = 0 ; pos < count ; ++pos) {
		if (getItem(pos) == item) {
			Select(pos, focus);
			break;
		}
	}
}
long ItemList::focusCount() const {
	long count = GetItemCount();
	long focused = 0;
	for (long pos = 0 ; pos < count ; ++pos) {
		if (const_cast<ItemList*>(this)->IsSelected(pos)) focused++;
	}
	return focused;
}

// ----------------------------------------------------------------------------- : ItemList : Building the list

// Comparison object for comparing items
struct ItemList::ItemComparer {
	ItemComparer(ItemList& list) : list(list) {}
	ItemList&   list; // 'this' pointer
	// Compare two items using the current criterium and order
	bool operator () (const VoidP& a, const VoidP& b) {
		if (list.sort_ascending) {
			return list.compareItems(a.get(), b.get());
		} else {
			return list.compareItems(b.get(), a.get());
		}
	}
};

void ItemList::refreshList(bool refresh_current_only) {
	// Get all items
	vector<VoidP> old_sorted_list;
	swap(sorted_list, old_sorted_list);
	getItems(sorted_list);
	// Sort the list
	if (sort_by_column >= 0) {
		sort(sorted_list.begin(), sorted_list.end(), ItemComparer(*this));
	}
	// Has the entire list changed?
	if (refresh_current_only && sorted_list == old_sorted_list) {
		if (selected_item_pos > 0) RefreshItem(selected_item_pos);
		return;
	}
	// refresh
	// Note: Freeze/Thaw makes flicker worse
	long item_count = (long)sorted_list.size();
	SetItemCount(item_count);
	// (re)select current item
	findSelectedItemPos();
	focusNone();
	focusSelectedItem(true);
	// refresh items
	if (item_count == 0) {
		Refresh();
	} else {
		RefreshItems(0, item_count - 1);
	}
}

void ItemList::sortBy(long column, bool ascending) {
	// Change image in column header
	long count = GetColumnCount();
	for (long i = 0 ; i < count ; ++i) {
		if (i == column) {
			SetColumnImage(i, sort_ascending ? 0 : 1); // arrow up/down
		} else if (i == sort_by_column) {
			SetColumnImage(i, -1);
		}
	}
	// sort list
	sort_by_column = column;
	sort_ascending = ascending;
	refreshList();
}

void ItemList::SetColumnImage(int col, int image) {
	// The wx version of this function is broken,
	// setting the wxLIST_MASK_IMAGE also sets the FORMAT flag, so we lose alignment info
	wxListItem item;
	item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_FORMAT);
	GetColumn(col, item);
    item.SetImage(image);
    SetColumn(col, item);
}

// ----------------------------------------------------------------------------- : ItemList : Window events

void ItemList::onColumnClick(wxListEvent& ev) {
	long new_sort_by_column = ev.GetColumn();
	if (sort_by_column == new_sort_by_column) {
		if (sort_ascending) {
			sort_ascending = false;		// 2nd click on same column -> sort descending
		} else if (mustSort()) {
			sort_ascending = true;		// 3rd click on same column -> sort ascending again
		} else {
			new_sort_by_column = -1;	// 3rd click on same column -> don't sort
		}
	} else {
		sort_ascending = true;
	}
	sortBy(new_sort_by_column, sort_ascending);
}

void ItemList::onItemFocus(wxListEvent& ev) {
	selectItemPos(ev.GetIndex(), false);
}

// ----------------------------------------------------------------------------- : ItemList : Event table

BEGIN_EVENT_TABLE(ItemList, wxListView)
	EVT_LIST_COL_CLICK		(wxID_ANY,	ItemList::onColumnClick)
	EVT_LIST_ITEM_FOCUSED	(wxID_ANY,	ItemList::onItemFocus)
END_EVENT_TABLE  ()
