//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/keyword_list.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/keyword.hpp>
#include <data/action/value.hpp>
#include <data/action/keyword.hpp>
#include <util/tagged_string.hpp>
#include <gfx/gfx.hpp>

DECLARE_TYPEOF_COLLECTION(KeywordP);

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_KEYWORD_SELECT);

// ----------------------------------------------------------------------------- : KeywordList

KeywordList::KeywordList(Window* parent, int id, long additional_style)
	: ItemList(parent, id, additional_style)
{
	// Add columns
	InsertColumn(0, _LABEL_("keyword"),  wxLIST_FORMAT_LEFT,    0);
	InsertColumn(1, _LABEL_("match"),    wxLIST_FORMAT_LEFT,  200);
	InsertColumn(2, _LABEL_("mode"),     wxLIST_FORMAT_LEFT,   60);
	InsertColumn(3, _LABEL_("uses"),     wxLIST_FORMAT_RIGHT,  80);
	InsertColumn(4, _LABEL_("reminder"), wxLIST_FORMAT_LEFT,  300);
}

KeywordList::~KeywordList() {
	storeColumns();
}
void KeywordList::storeColumns() {
	// TODO
}

void KeywordList::onBeforeChangeSet() {
	storeColumns();
}
void KeywordList::onChangeSet() {
	refreshList();
}

void KeywordList::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, AddKeywordAction) {
		if (undone) {
			long pos = selected_item_pos;
			refreshList();
			if (action.keyword == selected_item) {
				// select the next keyword, if not possible, select the last
				if (pos + 1 < GetItemCount()) {
					selectItemPos(pos, true);
				} else {
					selectItemPos(GetItemCount() - 1, true);
				}
			}
		} else {
			// select the new keyword
			selectItem(action.keyword, false /*list will be refreshed anyway*/, true);
			refreshList();
		}
	}
	TYPE_CASE(action, RemoveKeywordAction) {
		if (undone) {
			// select the re-added keyword
			selectItem(action.keyword, false /*list will be refreshed anyway*/, true);
			refreshList();
		} else {
			long pos = selected_item_pos;
			refreshList();
			if (action.keyword == selected_item) {
				// select the next keyword, if not possible, select the last
				if (pos + 1 < GetItemCount()) {
					selectItemPos(pos, true);
				} else {
					selectItemPos(GetItemCount() - 1, true);
				}
			}
		}
	}
	TYPE_CASE(action, ValueAction) {
		KeywordTextValue* value = dynamic_cast<KeywordTextValue*>(action.valueP.get());
		if (value) {
			// this is indeed an action on a keyword, refresh
			refreshList();
		}
	}
}

// ----------------------------------------------------------------------------- : KeywordListBase : for ItemList

String match_string(const Keyword& a) {
	return untag(replace_all(replace_all(
	            a.match,
		         _("<atom-param>"), _("‹")),
		        _("</atom-param>"), _("›"))
	       );
}

void KeywordList::getItems(vector<VoidP>& out) const {
	FOR_EACH(k, set->keywords) {
		k->fixed = false;
		out.push_back(k);
	}
	FOR_EACH(k, set->game->keywords) {
		k->fixed = true;
		out.push_back(k);
	}
}
void KeywordList::sendEvent() {
	KeywordSelectEvent ev(getKeyword());
	ProcessEvent(ev);
}
bool KeywordList::compareItems(void* a, void* b) const {
	const Keyword& ka = *(Keyword*)a;
	const Keyword& kb = *(Keyword*)b;
	switch(sort_by_column) {
		case 0:	return ka.keyword < kb.keyword;
		case 1: return ka.match   < kb.match;
		case 2: return ka.mode    < kb.mode;
		//case 3:
		case 4: return ka.reminder.getUnparsed() < kb.reminder.getUnparsed();
		default: // TODO: 3
				return ka.keyword < kb.keyword;
	}
}

// ----------------------------------------------------------------------------- : KeywordListBase : Item text

String KeywordList::OnGetItemText (long pos, long col) const {
	const Keyword& kw = *getKeyword(pos);
	switch(col) {
		case 0:		return kw.keyword;
		case 1:		return match_string(kw);
		case 2:		return kw.mode;
		case 3:		return _("TODO");
		case 4:		return kw.reminder.getUnparsed();
		default:	return wxEmptyString;
	}
}
int KeywordList::OnGetItemImage(long pos) const {
	return -1;
}

wxListItemAttr* KeywordList::OnGetItemAttr(long pos) const {
	// black for set keywords, grey for game keywords (uneditable)
	const Keyword& kw = *getKeyword(pos);
	if (!kw.fixed) return nullptr;
	item_attr.SetTextColour(lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),0.5));
	return &item_attr;
}
