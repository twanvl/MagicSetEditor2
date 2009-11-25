//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_list.hpp>
#include <gui/control/card_list_column_select.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <data/field/choice.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <data/stylesheet.hpp>
#include <data/format/clipboard.hpp>
#include <data/action/set.hpp>
#include <data/action/value.hpp>
#include <util/window_id.hpp>
#include <wx/clipbrd.h>

DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_POINTER_TYPE(ChoiceValue);
DECLARE_TYPEOF(map<int COMMA FieldP>);
DECLARE_TYPEOF_NO_REV(IndexMap<FieldP COMMA StyleP>);
DECLARE_TYPEOF_COLLECTION(CardListBase*);

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_CARD_SELECT);
DEFINE_EVENT_TYPE(EVENT_CARD_ACTIVATE);

CardP CardSelectEvent::getCard() const {
	return getTheCardList()->getCard();
}

void CardSelectEvent::getSelection(vector<CardP>& out) const {
	getTheCardList()->getSelection(out);
}

CardListBase* CardSelectEvent::getTheCardList() const {
	return static_cast<CardListBase*>(GetEventObject());
}

// ----------------------------------------------------------------------------- : CardListBase

vector<CardListBase*> CardListBase::card_lists;

CardListBase::CardListBase(Window* parent, int id, long additional_style)
	: ItemList(parent, id, additional_style, true)
{
	// add to the list of card lists
	card_lists.push_back(this);
}

CardListBase::~CardListBase() {
	storeColumns();
	// remove from list of card lists
	card_lists.erase(remove(card_lists.begin(), card_lists.end(), this));
}

void CardListBase::onBeforeChangeSet() {
	storeColumns();
}
void CardListBase::onChangeSet() {
	rebuild();
}

struct Freezer{
	Window* window;
	Freezer(Window* window) : window(window) { window->Freeze(); }
	~Freezer()                               { window->Thaw(); }
};

void CardListBase::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, AddCardAction) {
		Freezer freeze(this);
		if (action.action.adding != undone) {
			// select the new cards
			focusNone();
			selectItem(action.action.steps.front().item, false, true);
			refreshList();
			FOR_EACH_CONST(s, action.action.steps) focusItem(s.item); // focus all the new cards
		} else {
			long pos = selected_item_pos;
			// adjust focus for all the removed cards
			refreshList();
			if (!allowModify()) {
				// Let some other card list do the selecting, otherwise we get conflicting events
				return;
			}
			if (selected_item_pos == -1) {
				// selected item was deleted, select something else
				selectItemPos(pos, true, true);
			}
		}
	}
	TYPE_CASE(action, ReorderCardsAction) {
		if (sort_by_column >= 0) return; // nothing changes for us
                if ((long)action.card_id1 < 0 || (long)action.card_id2 >= sorted_list.size()) return;
		if ((long)action.card_id1 == selected_item_pos || (long)action.card_id2 == selected_item_pos) {
			// Selected card has moved; also move in the sorted card list
			swap(sorted_list[action.card_id1], sorted_list[action.card_id2]);
			// reselect the current card, it has moved
			selected_item_pos = (long)action.card_id1 == selected_item_pos ? (long)action.card_id2 : (long)action.card_id1;
			// select the right card
			focusSelectedItem();
		}
		RefreshItem((long)action.card_id1);
		RefreshItem((long)action.card_id2);
	}
	TYPE_CASE_(action, ScriptValueEvent) {
		// No refresh needed, a ScriptValueEvent is only generated in response to a ValueAction
		return;
	}
	TYPE_CASE(action, ValueAction) {
		if (action.card) refreshList(true);
	}
}

void CardListBase::getItems(vector<VoidP>& out) const {
	FOR_EACH(c, set->cards) {
		out.push_back(c);
	}
}
void CardListBase::sendEvent(int type) {
	CardSelectEvent ev(type);
	ev.SetEventObject(this);
	ProcessEvent(ev);
}

void CardListBase::getSelection(vector<CardP>& out) const {
	long count = GetItemCount();
	for (long pos = 0 ; pos < count ; ++pos) {
		if (const_cast<CardListBase*>(this)->IsSelected(pos)) {
			out.push_back(getCard(pos));
		}
	}
}

// ----------------------------------------------------------------------------- : CardListBase : Clipboard

bool CardListBase::canCut()   const { return canDelete(); }
bool CardListBase::canCopy()  const { return focusCount() > 0; }
bool CardListBase::canPaste() const {
	return allowModify() && wxTheClipboard->IsSupported(CardsDataObject::format);
}
bool CardListBase::canDelete() const {
	return allowModify() && focusCount() > 0; // TODO: check for selection?
}

bool CardListBase::doCopy() {
	if (!canCopy()) return false;
	// cards to copy
	vector<CardP> cards_to_copy;
	getSelection(cards_to_copy);
	if (cards_to_copy.empty()) return false;
	// put on clipboard
	if (!wxTheClipboard->Open()) return false;
	bool ok = wxTheClipboard->SetData(new CardsOnClipboard(set, cards_to_copy)); // ignore result
	wxTheClipboard->Close();
	return ok;
}
bool CardListBase::doPaste() {
	// get data
	if (!canPaste()) return false;
	if (!wxTheClipboard->Open()) return false;
	CardsDataObject data;
	bool ok = wxTheClipboard->GetData(data);
	wxTheClipboard->Close();
	if (!ok) return false;
	// get cards
	vector<CardP> new_cards;
	ok = data.getCards(set, new_cards);
	if (!ok) return false;
	// add card to set
	set->actions.addAction(new AddCardAction(ADD, *set, new_cards));
	return true;
}
bool CardListBase::doDelete() {
	// cards to delete
	vector<CardP> cards_to_delete;
	getSelection(cards_to_delete);
	if (cards_to_delete.empty()) return false;
	// delete cards
	set->actions.addAction(new AddCardAction(REMOVE, *set, cards_to_delete));
	return true;
}

// ----------------------------------------------------------------------------- : CardListBase : Building the list

// Comparison object for comparing cards
bool CardListBase::compareItems(void* a, void* b) const {
	FieldP sort_field = column_fields[sort_by_column];
	ValueP va = reinterpret_cast<Card*>(a)->data[sort_field];
	ValueP vb = reinterpret_cast<Card*>(b)->data[sort_field];
	assert(va && vb);
	// compare sort keys
	int cmp = smart_compare( va->getSortKey(), vb->getSortKey() );
	if (cmp != 0) return cmp < 0;
	// equal values, compare alternate sort key
	if (alternate_sort_field) {
		ValueP va = reinterpret_cast<Card*>(a)->data[alternate_sort_field];
		ValueP vb = reinterpret_cast<Card*>(b)->data[alternate_sort_field];
		int cmp = smart_compare( va->getSortKey(), vb->getSortKey() );
		if (cmp != 0) return cmp < 0;
	}
	return false;
}

void CardListBase::rebuild() {
	ClearAll();
	column_fields.clear();
	selected_item_pos = -1;
	onRebuild();
	if (!set) return;
	// init stuff
	set->game->initCardListColorScript();
	// determine column order
	map<int,FieldP> new_column_fields;
	FOR_EACH(f, set->game->card_fields) {
		ColumnSettings& cs = settings.columnSettingsFor(*set->game, *f);
		if (cs.visible && f->card_list_allow) {
			new_column_fields[cs.position] = f;
		}
	}
	// add columns
	FOR_EACH(f, new_column_fields) {
		ColumnSettings& cs = settings.columnSettingsFor(*set->game, *f.second);
		int align;
		if      (f.second->card_list_align & ALIGN_RIGHT)  align = wxLIST_FORMAT_RIGHT;
		else if (f.second->card_list_align & ALIGN_CENTER) align = wxLIST_FORMAT_CENTRE;
		else                                               align = wxLIST_FORMAT_LEFT;
		InsertColumn((long)column_fields.size(),
		             tr(*set->game, f.second->card_list_name, capitalize),
		             align, cs.width);
		column_fields.push_back(f.second);
	}
	// determine sort settings
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	sort_ascending = gs.sort_cards_ascending;
	sort_by_column = -1;
	long i = 0;
	FOR_EACH(f, column_fields) {
		if (f->name == gs.sort_cards_by) {
			// we are sorting by this column
			sort_by_column = i;
			// and display an arrow in the header
			SetColumnImage(i, sort_ascending ? 0 : 1);
		}
		++i;
	}
	// determine alternate sortImageFieldP ImageCardList::findImageField() {
	alternate_sort_field = FieldP();
	FOR_EACH(f, set->game->card_fields) {
		if (f->identifying) {
			alternate_sort_field = f;
			break;
		}
	}
	// refresh
	refreshList();
}

void CardListBase::sortBy(long column, bool ascending) {
	// sort all card lists for this game
	FOR_EACH(card_list, card_lists) {
		if (card_list->set && card_list->set->game == set->game) {
			card_list->ItemList::sortBy(column, ascending);
		}
	}
}

// ----------------------------------------------------------------------------- : CardListBase : Columns

void CardListBase::storeColumns() {
	if (!set) return;
	// store column widths
	int i = 0;
	FOR_EACH(f, column_fields) {
		ColumnSettings& cs = settings.columnSettingsFor(*set->game, *f);
		cs.width = GetColumnWidth(i++);
	}
	// store sorting
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	if (sort_by_column >= 0) gs.sort_cards_by = column_fields.at(sort_by_column)->name;
	else                     gs.sort_cards_by = wxEmptyString;
	gs.sort_cards_ascending = sort_ascending;
}
void CardListBase::selectColumns() {
	CardListColumnSelectDialog wnd(this, set->game);
	if (wnd.ShowModal() == wxID_OK) {
		// rebuild all card lists for this game
		storeColumns();
		FOR_EACH(card_list, card_lists) {
			if (card_list->set && card_list->set->game == set->game) {
				card_list->rebuild();
			}
		}
	}
}

// ----------------------------------------------------------------------------- : CardListBase : Item 'events'

String CardListBase::OnGetItemText(long pos, long col) const {
	if (col < 0 || (size_t)col >= column_fields.size()) {
		// wx may give us non existing columns!
		return wxEmptyString;
	}
	ValueP val = getCard(pos)->data[column_fields[col]];
	if (val) return val->toString();
	else     return wxEmptyString;
}

int CardListBase::OnGetItemImage(long pos) const {
	return -1;
}

wxListItemAttr* CardListBase::OnGetItemAttr(long pos) const {
	if (!set->game->card_list_color_script) return nullptr;
	Context& ctx = set->getContext(getCard(pos));
	item_attr.SetTextColour(*set->game->card_list_color_script.invoke(ctx));
	return &item_attr;
}

// ----------------------------------------------------------------------------- : CardListBase : Window events

void CardListBase::onColumnRightClick(wxListEvent&) {
	// show menu
	wxMenu m;
	m.Append(ID_SELECT_COLUMNS, _MENU_("card list columns"), _HELP_("card list columns"));
	PopupMenu(&m);
}
void CardListBase::onColumnResize(wxListEvent& ev) {
	storeColumns();
	int col = ev.GetColumn();
	int width = GetColumnWidth(col);
	FOR_EACH(card_list, card_lists) {
		if (card_list != this && card_list->set && card_list->set->game == set->game) {
			card_list->SetColumnWidth(col, width);
		}
	}
}

void CardListBase::onSelectColumns(wxCommandEvent&) {
	selectColumns();
}

void CardListBase::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_DELETE && allowModify()) {
		doDelete();
	} else if (ev.GetKeyCode() == WXK_TAB) {
		// send a navigation event to our parent, to select another control
		// we need this because tabs are not handled on the cards panel
		wxNavigationKeyEvent nev;
		nev.SetDirection(!ev.ShiftDown());
		GetParent()->ProcessEvent(nev);
	} else {
		ev.Skip();
	}
}

void CardListBase::onDrag(wxMouseEvent& ev) {
	if (!allowModify()) return;
	if (ev.Dragging() && selected_item && sort_by_column < 0) {
		// reorder card list
		int flags;
		long item = HitTest(ev.GetPosition(), flags);
		if (flags & wxLIST_HITTEST_ONITEM) {
			if (item > 0)                EnsureVisible(item-1);
			if (item < GetItemCount()-1) EnsureVisible(item+1);
			findSelectedItemPos();
			if (item != selected_item_pos) {
				// move card in the set
				set->actions.addAction(new ReorderCardsAction(*set, item, selected_item_pos));
			}
		}
	}
}

void CardListBase::onContextMenu(wxContextMenuEvent&) {
	if (allowModify()) {
		IconMenu m;
		m.Append(wxID_CUT,		_("cut"),		_CONTEXT_MENU_("cut"),			_HELP_("cut card"));
		m.Append(wxID_COPY,		_("copy"),		_CONTEXT_MENU_("copy"),			_HELP_("copy card"));
		m.Append(wxID_PASTE,	_("paste"),		_CONTEXT_MENU_("paste"),		_HELP_("paste card"));
		m.AppendSeparator();
		m.Append(ID_CARD_ADD,	_("card_add"),	_CONTEXT_MENU_("add card"),		_HELP_("add card"));
		m.Append(ID_CARD_REMOVE,_("card_del"),	_CONTEXT_MENU_("remove card"),	_HELP_("remove card"));
		PopupMenu(&m);
	}
}

void CardListBase::onItemActivate(wxListEvent& ev) {
	selectItemPos(ev.GetIndex(), false);
	sendEvent(EVENT_CARD_ACTIVATE);
}

// ----------------------------------------------------------------------------- : CardListBase : Event table

BEGIN_EVENT_TABLE(CardListBase, ItemList)
	EVT_LIST_COL_RIGHT_CLICK	(wxID_ANY,			CardListBase::onColumnRightClick)
	EVT_LIST_COL_END_DRAG		(wxID_ANY,			CardListBase::onColumnResize)
	EVT_LIST_ITEM_ACTIVATED		(wxID_ANY,			CardListBase::onItemActivate)
	EVT_CHAR					(					CardListBase::onChar)
	EVT_MOTION					(					CardListBase::onDrag)
	EVT_MENU					(ID_SELECT_COLUMNS,	CardListBase::onSelectColumns)
	EVT_CONTEXT_MENU            (                   CardListBase::onContextMenu)
END_EVENT_TABLE  ()
