//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_list.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <data/field/choice.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <data/action/set.hpp>
#include <util/window_id.hpp>

DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_POINTER_TYPE(ChoiceValue);
typedef map<int,FieldP> map_int_FieldP;
DECLARE_TYPEOF(map_int_FieldP);

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_CARD_SELECT);

// ----------------------------------------------------------------------------- : CardListBase

CardListBase::CardListBase(Window* parent, int id, int additional_style)
	: wxListView(parent, id, wxDefaultPosition, wxDefaultSize, additional_style | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
{
	// create image list
	wxImageList* il = new wxImageList(18,14);
	il->Add(Bitmap(_("SORT_ASC")),  Color(255,0,255));
	il->Add(Bitmap(_("SORT_DESC")), Color(255,0,255));
	AssignImageList(il, wxIMAGE_LIST_SMALL);
}

CardListBase::~CardListBase() {
	storeColumns();
}

void CardListBase::onBeforeChangeSet() {
	storeColumns();
}
void CardListBase::onChangeSet() {
	rebuild();
}

void CardListBase::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, AddCardAction) {
		// select the new card
		selectCard(action.card, false /*list will be refreshed anyway*/);
		refreshList();
	}
	TYPE_CASE(action, RemoveCardAction) {
		if (undone) {
			// select the re-added card
			selectCard(action.card, false /*list will be refreshed anyway*/);
			refreshList();
		} else {
			long pos = selected_card_pos;
			refreshList();
			if (action.card == selected_card) {
				// select the next card, if not possible, select the last
				if ((size_t)pos + 1 < sorted_card_list.size()) {
					selectCardPos(pos, true);
				} else {
					selectCardPos((long)sorted_card_list.size() - 1, true);
				}
			}
		}
	}
	TYPE_CASE(action, ReorderCardsAction) {
		if (sort_criterium) return; // nothing changes for us
		if ((long)action.card_id1 == selected_card_pos || (long)action.card_id2 == selected_card_pos) {
			// Selected card has moved; also move in the sorted card list
			swap(sorted_card_list[action.card_id1] ,sorted_card_list[action.card_id2]);
			// reselect the current card, it has moved
			selected_card_pos = (long)action.card_id1 == selected_card_pos ? (long)action.card_id2 : (long)action.card_id1;
			// select the right card
			selectCurrentCard();
		}
		RefreshItem((long)action.card_id1);
		RefreshItem((long)action.card_id2);
	}
}

vector<CardP>& CardListBase::getCards() const {
	return set->cards;
}

// ----------------------------------------------------------------------------- : CardListBase : Selection

bool CardListBase::canSelectPrevious() const {
	return selected_card_pos - 1 >= 0;
}
bool CardListBase::canSelectNext() const {
	return selected_card_pos >= 0 && static_cast<size_t>(selected_card_pos + 1) < sorted_card_list.size();
}
void CardListBase::selectPrevious() {
	assert(selected_card_pos >= 1);
	selectCardPos(selected_card_pos - 1, true);
}
void CardListBase::selectNext() {
	assert(selected_card_pos + 1 < (long)sorted_card_list.size());
	selectCardPos(selected_card_pos + 1, true);
}

// ----------------------------------------------------------------------------- : CardListBase : Selection (private)

void CardListBase::selectCard(const CardP& card, bool focus) {
	selected_card = card;
	CardSelectEvent ev(card);
	ProcessEvent(ev);
	if (focus) {
		findSelectedCardPos();
		selectCurrentCard();
	}
}

void CardListBase::selectCardPos(long pos, bool focus) {
	if (selected_card_pos == pos && !focus)  return; // this card is already selected
	if ((size_t)pos < sorted_card_list.size()) {
		// only if there is something to select
		selectCard(sorted_card_list[pos], false);
	} else {
		selectCard(CardP(), false);
	}
	selected_card_pos = pos;
	if (focus) selectCurrentCard();
}

void CardListBase::findSelectedCardPos() {
	// find the position of the selected card
	long count = GetItemCount();
	selected_card_pos = -1;
	for (long pos = 0 ; pos < count ; ++pos) {
		if (sorted_card_list[pos] == selected_card) {
			selected_card_pos = pos;
			break;
		}
	}
}
void CardListBase::selectCurrentCard() {
	if (GetItemCount() > 0) {
		if (selected_card_pos == -1 || (size_t)selected_card_pos > sorted_card_list.size()) {
			// deselect currently selected item, if any
			long sel = GetFirstSelected();
			Select(sel, false);
		} else {
			Select(selected_card_pos);
			Focus(selected_card_pos);
		}
	}
}

// ----------------------------------------------------------------------------- : CardListBase : Building the list

// Comparison object for comparing cards
struct CardListBase::CardComparer {
	CardComparer(CardListBase& cl) : cl(cl) {}
	CardListBase& cl; // 'this' pointer
	// Compare two cards using the current criterium and order
	bool operator () (const CardP& a, const CardP& b) {
		ValueP va = a->data[cl.sort_criterium];
		ValueP vb = b->data[cl.sort_criterium];
		if (cl.sort_ascending) {
			if (!va || !vb)  return va < vb; // got to do something, compare pointers
			return smart_less(  va->toString() , vb->toString() );
		} else {
			if (!va || !vb)  return vb < va;
			return smart_less(  vb->toString() , va->toString() );
		}
	}
};

void CardListBase::sortList() {
	sorted_card_list.clear();
	FOR_EACH(card, getCards()) {
		sorted_card_list.push_back(card);
	}
	if (sort_criterium) {
		sort(sorted_card_list.begin(), sorted_card_list.end(), CardComparer(*this));
	}
}

void CardListBase::rebuild() {
	ClearAll();
	column_fields.clear();
	selected_card_pos = -1;
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
		if      (f.second->card_list_align == ALIGN_RIGHT)  align = wxLIST_FORMAT_RIGHT;
		else if (f.second->card_list_align == ALIGN_CENTER) align = wxLIST_FORMAT_CENTRE;
		else                                                align = wxLIST_FORMAT_LEFT;
		InsertColumn((long)column_fields.size(), capitalize(f.second->card_list_name), align, cs.width);
		column_fields.push_back(f.second);
	}
	// find field that determines color
	color_style = findColorStyle();
	// determine sort settings
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	sort_ascending = gs.sort_cards_ascending;
	sort_criterium = FieldP();
	int i = 0;
	FOR_EACH(f, column_fields) {
		if (f->name == gs.sort_cards_by) {
			// we are sorting by this column, store the field
			sort_criterium = f;
			// and display an arrow in the header
			wxListItem li;
			li.m_mask  = wxLIST_MASK_IMAGE;
			li.m_image = sort_ascending ? 0 : 1; // arrow up/down
			SetColumn(i, li);
		}
	}
	refreshList();
	// select a card if possible
	selectCardPos(0, true);
}

void CardListBase::refreshList() {
	// ensure correct list size
	long items = (long) getCards().size();
	SetItemCount(items);
	// (re)sort the list
	sortList();
	// refresh
	RefreshItems(0, items - 1);
	if (items == 0) Refresh();
	// select
	findSelectedCardPos();
	selectCurrentCard();
}

ChoiceStyleP CardListBase::findColorStyle() {
/*	FOR_EACH(s, set->default_stylesheet->card_style) {
		ChoiceStyleP cs = dynamic_cast<ChoiceStyleP>(s);
		if (cs && cs->colors_card_list) {
			return cs;
		}
	}
*/
	return ChoiceStyleP();
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
	if (sort_criterium) gs.sort_cards_by = sort_criterium->name;
	else                gs.sort_cards_by = wxEmptyString;
	gs.sort_cards_ascending = sort_ascending;
}
void CardListBase::selectColumns() {
//	CardListColumnSelect wnd(this, set->game);
//	if (wnd.ShowModal() == wxID_OK) {
//		rebuild(); // columns have changed
//	}
}

// ----------------------------------------------------------------------------- : CardListBase : Item 'events'

String CardListBase::OnGetItemText(long pos, long col) const {
	if (col < 0 || (size_t)col >= column_fields.size()) {
		// wx may give us non existing columns!
		return wxEmptyString;
	}
	ValueP val = sorted_card_list[pos]->data[column_fields[col]];
	if (val) return val->toString();
	else     return wxEmptyString;
}

int CardListBase::OnGetItemImage(long pos) const {
	return -1;
}

wxListItemAttr* CardListBase::OnGetItemAttr(long pos) const {
	if (!color_style) return nullptr;
	ChoiceValueP val = static_pointer_cast<ChoiceValue>( sorted_card_list[pos]->data[color_style->fieldP]);
	assert(val);
	item_attr.SetTextColour(color_style->choice_colors[val->value()]); // if it doesn't exist we get black
	return &item_attr;
}

// ----------------------------------------------------------------------------- : CardListBase : Window events

void CardListBase::onColumnClick(wxListEvent& ev) {
	FieldP new_sort_criterium = column_fields[ev.GetColumn()];
	if (sort_criterium == new_sort_criterium) {
		if (sort_ascending) {
			sort_ascending = false;			// 2nd click on same column -> sort descending
		} else {
			new_sort_criterium.reset();		// 3rd click on same column -> don't sort
		}
	} else {
		sort_ascending = true;
	}
	// Change image in column header
	int i = 0;
	FOR_EACH(f, column_fields) {
		if (f == new_sort_criterium) {
			SetColumnImage(i, sort_ascending ? 0 : 1); // arrow up/down
		} else if (f == sort_criterium) {
			ClearColumnImage(i);
		}
		++i;
	}
	sort_criterium = new_sort_criterium;
	refreshList();
}

void CardListBase::onColumnRightClick(wxListEvent&) {
	// show menu
	wxMenu* m = new wxMenu;
	m->Append(ID_SELECT_COLUMNS, _("&Select Columns..."), _("Select what columns should be shown and in what order."));
	PopupMenu(m);
}

void CardListBase::onSelectColumns(wxCommandEvent&) {
	selectColumns();
}

void CardListBase::onItemFocus(wxListEvent& ev) {
	selectCardPos(ev.GetIndex(), false);
}

void CardListBase::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_DELETE) {
		set->actions.add(new RemoveCardAction(*set, selected_card));
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
// TODO
}

// ----------------------------------------------------------------------------- : CardListBase : Event table

BEGIN_EVENT_TABLE(CardListBase, wxListView)
	EVT_LIST_COL_CLICK			(wxID_ANY,			CardListBase::onColumnClick)
	EVT_LIST_COL_RIGHT_CLICK	(wxID_ANY,			CardListBase::onColumnRightClick)
	EVT_LIST_ITEM_FOCUSED		(wxID_ANY,			CardListBase::onItemFocus)
	EVT_CHAR					(					CardListBase::onChar)
	EVT_MOTION					(					CardListBase::onDrag)
	EVT_MENU					(ID_SELECT_COLUMNS,	CardListBase::onSelectColumns)
END_EVENT_TABLE  ()
