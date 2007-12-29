//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/item_list.hpp>
#include <gui/util.hpp>
#include <wx/imaglist.h>

// ----------------------------------------------------------------------------- : ItemList

ItemList::ItemList(Window* parent, int id, long additional_style)
	: wxListView(parent, id, wxDefaultPosition, wxDefaultSize, additional_style | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
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
	selectItemPos(selected_item_pos - 1, true);
}
void ItemList::selectNext() {
	assert(selected_item_pos + 1 < (long)sorted_list.size());
	selectItemPos(selected_item_pos + 1, true);
}
void ItemList::selectFirst() {
	assert(0 < (long)sorted_list.size());
	selectItemPos(0, true);
}

// ----------------------------------------------------------------------------- : ItemList : Selection (private)

void ItemList::selectItem(const VoidP& item, bool focus, bool event) {
	selected_item = item;
	if (event) sendEvent();
	findSelectedItemPos();
	if (focus) {
		selectCurrentItem();
	}
}

void ItemList::selectItemPos(long pos, bool focus) {
	if (selected_item_pos == pos && !focus) return; // this item is already selected
	if ((size_t)pos < sorted_list.size()) {
		// only if there is something to select
		selectItem(getItem(pos), false, true);
	} else {
		selectItem(VoidP(),      false, true);
	}
	selected_item_pos = pos;
	if (focus) selectCurrentItem();
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
void ItemList::selectCurrentItem() {
	if (GetItemCount() > 0) {
		if (selected_item_pos == -1 || (size_t)selected_item_pos > sorted_list.size()) {
			// deselect currently selected item, if any
			long sel = GetFirstSelected();
			Select(sel, false);
		} else {
			Select(selected_item_pos);
			Focus (selected_item_pos);
		}
	}
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

void ItemList::refreshList() {
	// Get all items
	sorted_list.clear();
	getItems(sorted_list);
	long item_count = (long)sorted_list.size();
	SetItemCount(item_count);
	// Sort the list
	if (sort_by_column >= 0) {
		sort(sorted_list.begin(), sorted_list.end(), ItemComparer(*this));
	}
	// refresh
	RefreshItems(0, item_count - 1);
	if (item_count == 0) Refresh();
	// (re)select current item
	findSelectedItemPos();
	selectCurrentItem();
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
