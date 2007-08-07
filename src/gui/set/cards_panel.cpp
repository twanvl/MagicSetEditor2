//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/cards_panel.hpp>
#include <gui/control/image_card_list.hpp>
#include <gui/control/card_editor.hpp>
#include <gui/control/text_ctrl.hpp>
#include <gui/about_window.hpp>
#include <gui/update_checker.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/action/set.hpp>
#include <data/settings.hpp>
#include <util/find_replace.hpp>
#include <util/tagged_string.hpp>
#include <util/window_id.hpp>
#include <wx/splitter.h>

// ----------------------------------------------------------------------------- : CardsPanel

CardsPanel::CardsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id, false)
{
	// init controls
	wxPanel* notesP;
	editor    = new CardEditor(this, ID_EDITOR);
	splitter  = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	card_list = new ImageCardList(splitter, ID_CARD_LIST);
	notesP    = new Panel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 /* no tab traversal*/);
	notes     = new TextCtrl(notesP, ID_NOTES, true);
	collapse_notes = new HoverButton(notesP, ID_COLLAPSE_NOTES, _("btn_collapse"), wxNullColour);
	collapse_notes->SetExtraStyle(wxWS_EX_PROCESS_UI_UPDATES);
	// init sizer for notes panel
	wxSizer* sn = new wxBoxSizer(wxVERTICAL);
		wxSizer* sc = new wxBoxSizer(wxHORIZONTAL);
		sc->Add(new wxStaticText(notesP, wxID_ANY, _LABEL_("card notes")), 1, wxEXPAND);
		sc->Add(collapse_notes, 0, wxALIGN_CENTER | wxRIGHT, 2);
	sn->Add(sc, 0, wxEXPAND, 2);
	sn->Add(notes, 1, wxEXPAND | wxTOP, 2);
	notesP->SetSizer(sn);
	// init splitter
	splitter->SetMinimumPaneSize(15);
	splitter->SetSashGravity(1.0);
	splitter->SplitHorizontally(card_list, notesP, -40);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(editor,   0, wxRIGHT, 2);
	s->Add(splitter, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
	
	// init menus
	menuCard = new IconMenu();
		menuCard->Append(ID_CARD_PREV,								_MENU_("previous card"),	_HELP_("previous card"));
		menuCard->Append(ID_CARD_NEXT,								_MENU_("next card"),		_HELP_("next card"));
		menuCard->AppendSeparator();
		menuCard->Append(ID_CARD_ADD,		_("card_add"),			_MENU_("add card"),			_HELP_("add card"));
		menuCard->Append(ID_CARD_ADD_MULT,	_("card_add_multiple"),	_MENU_("add cards"),		_HELP_("add cards"));
																	// NOTE: space after "Del" prevents wx from making del an accellerator
																	// otherwise we delete a card when delete is pressed inside the editor
																	// Adding a space never hurts, please keep it just to be safe.
		menuCard->Append(ID_CARD_REMOVE,	_("card_del"),			_MENU_("remove card")+_(" "),_HELP_("remove card"));
		menuCard->AppendSeparator();
		IconMenu* menuRotate = new IconMenu();
			menuRotate->Append(ID_CARD_ROTATE_0,		_("card_rotate_0"),		_MENU_("rotate 0"),		_HELP_("rotate 0"),		wxITEM_CHECK);
			menuRotate->Append(ID_CARD_ROTATE_270,		_("card_rotate_270"),	_MENU_("rotate 270"),	_HELP_("rotate 270"),	wxITEM_CHECK);
			menuRotate->Append(ID_CARD_ROTATE_90,		_("card_rotate_90"),	_MENU_("rotate 90"),	_HELP_("rotate 90"),	wxITEM_CHECK);
			menuRotate->Append(ID_CARD_ROTATE_180,		_("card_rotate_180"),	_MENU_("rotate 180"),	_HELP_("rotate 180"),	wxITEM_CHECK);
		menuCard->Append(wxID_ANY,			_("card_rotate"),		_MENU_("orientation"),		_HELP_("orientation"),		wxITEM_NORMAL, menuRotate);
		menuCard->AppendSeparator();
		// This probably belongs in the window menu, but there we can't remove the separator once it is added
		menuCard->Append(ID_SELECT_COLUMNS,							_MENU_("card list columns"),_HELP_("card list columns"));
	
	menuFormat = new IconMenu();
		menuFormat->Append(ID_FORMAT_BOLD,		_("bold"),			_MENU_("bold"),				_HELP_("bold"),				wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_ITALIC,	_("italic"),		_MENU_("italic"),			_HELP_("italic"),			wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_SYMBOL,	_("symbol"),		_MENU_("symbols"),			_HELP_("symbols"),			wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_REMINDER,	_("reminder"),		_MENU_("reminder text"),	_HELP_("reminder text"),	wxITEM_CHECK);
		menuFormat->AppendSeparator();
		insertSymbolMenu = new wxMenuItem(menuFormat, ID_INSERT_SYMBOL, _MENU_("insert symbol"));
		menuFormat->Append(insertSymbolMenu);
}

CardsPanel::~CardsPanel() {
//	settings.card_notes_height = splitter->GetSashPosition();
	// we don't own the submenu
	wxMenu* menu = insertSymbolMenu->GetSubMenu();
	if (menu && menu->GetParent() == menuFormat) {
		menu->SetParent(nullptr);
	}
	insertSymbolMenu->SetSubMenu(nullptr); 
	// delete menus
	delete menuCard;
	delete menuFormat;
}

void CardsPanel::onChangeSet() {
	editor->setSet(set);
	notes->setSet(set);
	card_list->setSet(set);
/*	// resize editor
	Sizer* s = sizer;
	minSize = s->minSize;
	layout();*/
}

// ----------------------------------------------------------------------------- : UI

void CardsPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->AddTool(ID_FORMAT_BOLD,		_(""), load_resource_tool_image(_("bold")),			wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("bold"),			_HELP_("bold"));
	tb->AddTool(ID_FORMAT_ITALIC,	_(""), load_resource_tool_image(_("italic")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("italic"),		_HELP_("italic"));
	tb->AddTool(ID_FORMAT_SYMBOL,	_(""), load_resource_tool_image(_("symbol")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("symbols"),		_HELP_("symbols"));
	tb->AddTool(ID_FORMAT_REMINDER,	_(""), load_resource_tool_image(_("reminder")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("reminder text"),	_HELP_("reminder text"));
	tb->AddSeparator();
	tb->AddTool(ID_CARD_ADD,		_(""), load_resource_tool_image(_("card_add")),		wxNullBitmap, wxITEM_NORMAL,_TOOLTIP_("add card"),		_HELP_("add card"));
	tb->AddTool(ID_CARD_REMOVE,		_(""), load_resource_tool_image(_("card_del")),		wxNullBitmap, wxITEM_NORMAL,_TOOLTIP_("remove card"),	_HELP_("remove card"));
	tb->AddSeparator();
	tb->AddTool(ID_CARD_ROTATE,		_(""), load_resource_tool_image(_("card_rotate")),	wxNullBitmap,wxITEM_NORMAL, _TOOLTIP_("rotate card"),	_HELP_("rotate card"));
	tb->Realize();
	// Menus
	mb->Insert(2, menuCard,   _MENU_("cards"));
	mb->Insert(3, menuFormat, _MENU_("format"));
}

void CardsPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->DeleteTool(ID_FORMAT_BOLD);
	tb->DeleteTool(ID_FORMAT_ITALIC);
	tb->DeleteTool(ID_FORMAT_SYMBOL);
	tb->DeleteTool(ID_FORMAT_REMINDER);
	tb->DeleteTool(ID_CARD_ADD);
	tb->DeleteTool(ID_CARD_REMOVE);
	tb->DeleteTool(ID_CARD_ROTATE);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(12); // delete separator
	tb->DeleteToolByPos(12); // delete separator
	// Menus
	mb->Remove(3);
	mb->Remove(2);
}

void CardsPanel::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_CARD_PREV:       ev.Enable(card_list->canSelectPrevious());	break;
		case ID_CARD_NEXT:       ev.Enable(card_list->canSelectNext());		break;
		case ID_CARD_ROTATE_0: case ID_CARD_ROTATE_90: case ID_CARD_ROTATE_180: case ID_CARD_ROTATE_270: {
			StyleSheetSettings& ss = settings.stylesheetSettingsFor(set->stylesheetFor(card_list->getCard()));
			int a = ev.GetId() == ID_CARD_ROTATE_0   ? 0
			      : ev.GetId() == ID_CARD_ROTATE_90  ? 90
			      : ev.GetId() == ID_CARD_ROTATE_180 ? 180
			      :                                    270;
			ev.Check(ss.card_angle() == a);
			break;
		}
		case ID_CARD_REMOVE:     ev.Enable(set->cards.size() > 1);			break;
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: case ID_FORMAT_REMINDER: {
			if (focused_control(this) == ID_EDITOR) {
				ev.Enable(editor->canFormat(ev.GetId()));
				ev.Check (editor->hasFormat(ev.GetId()));
			} else {
				ev.Enable(false);
				ev.Check(false);
			}
			break;
		}
		case ID_COLLAPSE_NOTES: {
			bool collapse = notes->GetSize().y > 0;
			collapse_notes->loadBitmaps(collapse ? _("btn_collapse") : _("btn_expand"));
			break;
		}
		case ID_INSERT_SYMBOL: {
			wxMenu* menu = editor->getMenu(ID_INSERT_SYMBOL);
			ev.Enable(menu);
			if (insertSymbolMenu->GetSubMenu() != menu  ||  (menu && menu->GetParent() != menuFormat)) {
				// re-add the menu
				menuFormat->Remove(insertSymbolMenu);
				insertSymbolMenu->SetSubMenu(menu);
				menuFormat->Append(insertSymbolMenu);
			}
		}
	}
}

void CardsPanel::onCommand(int id) {
	switch (id) {
		case ID_CARD_PREV:
			card_list->selectPrevious();
			break;
		case ID_CARD_NEXT:
			card_list->selectNext();
			break;
		case ID_CARD_ADD:
			set->actions.add(new AddCardAction(*set));
			break;
		case ID_CARD_REMOVE:
			if (card_list->getCard() != nullptr && set->cards.size() != 1)
				//Don't delete the last card, and certainly don't delete a card if none exists.
				set->actions.add(new RemoveCardAction(*set, card_list->getCard()));
			break;
		case ID_CARD_ROTATE:
		case ID_CARD_ROTATE_0: case ID_CARD_ROTATE_90: case ID_CARD_ROTATE_180: case ID_CARD_ROTATE_270: {
			StyleSheetSettings& ss = settings.stylesheetSettingsFor(set->stylesheetFor(card_list->getCard()));
			ss.card_angle.assign(
				  id == ID_CARD_ROTATE     ? (ss.card_angle() + 90) % 360
				: id == ID_CARD_ROTATE_0   ? 0
				: id == ID_CARD_ROTATE_90  ? 90
				: id == ID_CARD_ROTATE_180 ? 180
				:                            270
			);
			set->actions.tellListeners(DisplayChangeAction(),true);
			break;
		}
		case ID_SELECT_COLUMNS: {
			card_list->selectColumns();
		}
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: case ID_FORMAT_REMINDER: {
			if (focused_control(this) == ID_EDITOR) {
				editor->doFormat(id);
				break;
			}
		}
		case ID_COLLAPSE_NOTES: {
			bool collapse = notes->GetSize().y > 0;
			if (collapse) {
				splitter->SetSashPosition(-1);
				card_list->SetFocus();
			} else {
				splitter->SetSashPosition(-150);
				notes->SetFocus();
			}
			break;
		}
		default: {
			if (id >= ID_INSERT_SYMBOL_MENU_MIN && id <= ID_INSERT_SYMBOL_MENU_MAX) {
				// pass on to editor
				editor->onCommand(id);
			}
		}
	}
}

// ----------------------------------------------------------------------------- : Actions

bool CardsPanel::wantsToHandle(const Action&, bool undone) const {
	return false;
}

// ----------------------------------------------------------------------------- : Clipboard

// determine what control to use for clipboard actions
#define CUT_COPY_PASTE(op,return)									\
	int id = focused_control(this);									\
	if      (id == ID_EDITOR)    { return editor->op();    }		\
	else if (id == ID_CARD_LIST) { return card_list->op(); }		\
	else if (id == ID_NOTES)     { return notes->op();     }		\
	else                         { return false;           }

bool CardsPanel::canCut()   const { CUT_COPY_PASTE(canCut,   return) }
bool CardsPanel::canCopy()  const { CUT_COPY_PASTE(canCopy,  return) }
bool CardsPanel::canPaste() const { CUT_COPY_PASTE(canPaste, return) }
void CardsPanel::doCut()          { CUT_COPY_PASTE(doCut,    return (void)) }
void CardsPanel::doCopy()         { CUT_COPY_PASTE(doCopy,   return (void)) }
void CardsPanel::doPaste()        { CUT_COPY_PASTE(doPaste,  return (void)) }

// ----------------------------------------------------------------------------- : Searching

class CardsPanel::SearchFindInfo : public FindInfo {
  public:
	SearchFindInfo(CardsPanel& panel, wxFindReplaceData& what) : FindInfo(what), panel(panel) {}
	virtual bool handle(const CardP& card, const TextValueP& value, size_t pos, bool was_selection) {
		// Select the card
		panel.card_list->setCard(card);
		return true;
	}
  private:
	CardsPanel& panel;
};

class CardsPanel::ReplaceFindInfo : public FindInfo {
  public:
	ReplaceFindInfo(CardsPanel& panel, wxFindReplaceData& what) : FindInfo(what), panel(panel) {}
	virtual bool handle(const CardP& card, const TextValueP& value, size_t pos, bool was_selection) {
		// Select the card
		panel.card_list->setCard(card);
		// Replace
		if (was_selection) {
			panel.editor->insert(escape(what.GetReplaceString()), _("Replace"));
			return false;
		} else {
			return true;
		}
	}
	virtual bool searchSelection() const { return true; }
  private:
	CardsPanel& panel;
};

bool CardsPanel::doFind(wxFindReplaceData& what) {
	SearchFindInfo find(*this, what);
	return search(find, false);
}
bool CardsPanel::doReplace(wxFindReplaceData& what) {
	ReplaceFindInfo find(*this, what);
	return search(find, false);
}
bool CardsPanel::doReplaceAll(wxFindReplaceData& what) {
	return false; // TODO
}

bool CardsPanel::search(FindInfo& find, bool from_start) {
	bool include = from_start;
	CardP current = card_list->getCard();
	for (size_t i = 0 ; i < set->cards.size() ; ++i) {
		CardP card = card_list->getCard( (long) (find.forward() ? i : set->cards.size() - i - 1) );
		if (card == current) include = true;
		if (include) {
			editor->setCard(card);
			if (editor->search(find, from_start || card != current)) {
				return true; // done
			}
		}
	}
	editor->setCard(current);
	return false;
}

// ----------------------------------------------------------------------------- : Selection

CardP CardsPanel::selectedCard() const {
	return card_list->getCard();
}
void CardsPanel::selectCard(const CardP& card) {
	if (!set) return; // we want onChangeSet first
	card_list->setCard(card);
	editor->setCard(card);
	notes->setValue(card ? &card->notes : nullptr);
	Layout();
}

void CardsPanel::selectFirstCard() {
	if (!set) return; // we want onChangeSet first
	card_list->selectFirst();
}
