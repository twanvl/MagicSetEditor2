//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/part_list.hpp>
#include <gui/util.hpp>
#include <data/action/symbol.hpp>
#include <wx/imaglist.h>

// ----------------------------------------------------------------------------- : Constructor

SymbolPartList::SymbolPartList(Window* parent, int id, SymbolP symbol)
	: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
					wxLC_REPORT | wxLC_NO_HEADER | wxLC_VIRTUAL | wxLC_EDIT_LABELS)
	, SymbolView(symbol)
{
	// Create image list
	wxImageList* images = new wxImageList(16,16);
	// NOTE: this is based on the order of the SymbolPartCombine enum!
	images->Add(loadResourceImage(_("COMBINE_OR")));
	images->Add(loadResourceImage(_("COMBINE_SUB")));
	images->Add(loadResourceImage(_("COMBINE_AND")));
	images->Add(loadResourceImage(_("COMBINE_XOR")));
	images->Add(loadResourceImage(_("COMBINE_OVER")));
	images->Add(loadResourceImage(_("COMBINE_BORDER")));
	AssignImageList(images, wxIMAGE_LIST_SMALL);
	// create columns
	InsertColumn(0, _("Name"));
	update();
}

// ----------------------------------------------------------------------------- : View events

void SymbolPartList::onSymbolChange() {
	update();
}

void SymbolPartList::onAction(const Action& action) {
	TYPE_CASE(action, ReorderSymbolPartsAction) {
		if (selected == (long) action.partId1) {
			selectItem((long) action.partId2);
		} else if (selected == (long) action.partId2) {
			selectItem((long) action.partId1);
		}
	}
	TYPE_CASE_(action, SymbolPartListAction) {
		update();
	}
}

// ----------------------------------------------------------------------------- : Other

String SymbolPartList::OnGetItemText(long item, long col) const {
	assert(col == 0);
	return getPart(item)->name;
}
int SymbolPartList::OnGetItemImage(long item) const {
	return getPart(item)->combine;
}

SymbolPartP SymbolPartList::getPart(long item) const {
	return symbol->parts.at(item);
}
void SymbolPartList::selectItem(long item) {
	selected = (long)item;
	long count = GetItemCount();
	for (long i = 0 ; i < count ; ++i) {
		SetItemState(i, i == selected ? wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED : 0,
		                                wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
	}
}

void SymbolPartList::getSelectedParts(set<SymbolPartP>& sel) {
	sel.clear();
	long count = GetItemCount();
	for (long i = 0 ; i < count ; ++ i) {
		bool selected = GetItemState(i, wxLIST_STATE_SELECTED);
		if (selected) {
			sel.insert(symbol->parts.at(i));
		}
	}
}

void SymbolPartList::selectParts(const set<SymbolPartP>& sel) {
	long count = GetItemCount();
	for (long i = 0 ; i < count ; ++ i) {
		// is that part selected?
		bool selected = sel.find(symbol->parts.at(i)) != sel.end();
		SetItemState(i, selected ? wxLIST_STATE_SELECTED : 0,
			                       wxLIST_STATE_SELECTED);
	}	
}
	
void SymbolPartList::update() {
	if (symbol->parts.empty()) {
		// deleting all items requires a full refresh on win32
		SetItemCount(0);
		Refresh(true);
	} else {
		SetItemCount((long) symbol->parts.size() );
		Refresh(false);
	}
}

// ----------------------------------------------------------------------------- : Event handling

void SymbolPartList::onSelect(wxListEvent& ev) {
	selected = ev.GetIndex();
	ev.Skip();
}
void SymbolPartList::onDeselect(wxListEvent& ev) {
	selected = -1;
	ev.Skip();
}

void SymbolPartList::onLabelEdit(wxListEvent& ev){
	symbol->actions.add(
		new SymbolPartNameAction(getPart(ev.GetIndex()), ev.GetLabel())
	);
}

void SymbolPartList::onSize(wxSizeEvent& ev) {
	wxSize s = GetClientSize();
	SetColumnWidth(0, s.GetWidth() - 2);
}

void SymbolPartList::onDrag(wxMouseEvent& ev) {
	if (!ev.Dragging() || selected == -1) return;
	// reorder the list of parts
	int flags;
	long item = HitTest(ev.GetPosition(), flags);
	if (flags & wxLIST_HITTEST_ONITEM) {
		if (item > 0)                  EnsureVisible(item - 1);
		if (item < GetItemCount() - 1) EnsureVisible(item + 1);
		if (item != selected) {
			// swap the two items
			symbol->actions.add(new ReorderSymbolPartsAction(*symbol, item, selected));
			selectItem(item); // deselect all other items, to prevent 'lassoing' them
		}
	}
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(SymbolPartList, wxListCtrl)
	EVT_LIST_ITEM_SELECTED   (wxID_ANY,	SymbolPartList::onSelect)
	EVT_LIST_ITEM_DESELECTED (wxID_ANY,	SymbolPartList::onDeselect)
	EVT_LIST_END_LABEL_EDIT  (wxID_ANY,	SymbolPartList::onLabelEdit)
	EVT_SIZE                 (			SymbolPartList::onSize)
	EVT_MOTION               (			SymbolPartList::onDrag)
END_EVENT_TABLE  ()
