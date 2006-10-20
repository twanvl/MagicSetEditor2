//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/cards_panel.hpp>
#include <gui/control/card_list.hpp>
#include <gui/icon_menu.hpp>
#include <data/set.hpp>
#include <data/action/set.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <wx/splitter.h>

// ----------------------------------------------------------------------------- : CardsPanel

CardsPanel::CardsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id, false)
{
	// init controls
//	splitter = new SplitterWindow(&this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
//	card_list = new EditCardList(splitter, idCardList);
//	card_list = new EditCardList(splitter, ID_CARD_LIST);
	card_list = new CardListBase(this, ID_CARD_LIST);
	// init splitter
//	splitter->minimumPaneSize = 14;
//	splitter->sashGravity = 1.0;
//	splitter->splitHorizontally(cardList, notesP, -40);
	// init sizer
/*	Sizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(editor, 0, wxRIGHT, 2);
	s->Add(splitter, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
*/
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(card_list, 1, wxEXPAND);
	SetSizer(s);
}

CardsPanel::~CardsPanel() {
//	settings.card_notes_height = splitter->GetSashPosition();
}

void CardsPanel::onChangeSet() {
//	editor->setSet(set);
	card_list->setSet(set);
/*	// resize editor
	Sizer* s = sizer;
	minSize = s->minSize;
	layout();*/
}

// ----------------------------------------------------------------------------- : UI

void CardsPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->AddTool(ID_FORMAT_BOLD,		_(""), Bitmap(_("TOOL_BOLD")),		wxNullBitmap, wxITEM_CHECK, _("Bold"));
	tb->AddTool(ID_FORMAT_ITALIC,	_(""), Bitmap(_("TOOL_ITALIC")),	wxNullBitmap, wxITEM_CHECK, _("Italic"));
	tb->AddTool(ID_FORMAT_SYMBOL,	_(""), Bitmap(_("TOOL_SYMBOL")),	wxNullBitmap, wxITEM_CHECK, _("Symbols"));
	tb->AddSeparator();
	tb->AddTool(ID_CARD_ADD,		_(""), Bitmap(_("TOOL_CARD_ADD")),	wxNullBitmap, wxITEM_NORMAL,_("Add card"));
	tb->AddTool(ID_CARD_REMOVE,		_(""), Bitmap(_("TOOL_CARD_DEl")),	wxNullBitmap, wxITEM_NORMAL,_("Remove selected card"));
	tb->AddSeparator();
	tb->AddTool(ID_CARD_ROTATE,		_(""), Bitmap(_("TOOL_CARD_ROTATE")),wxNullBitmap,wxITEM_NORMAL,_("Rotate card"));
	tb->Realize();
	// Menus
	IconMenu* menuCard = new IconMenu();
		menuCard->Append(ID_CARD_PREV,								_("Select &Previous Card\tPgUp"),	_("Selects the previous card in the list"));
		menuCard->Append(ID_CARD_NEXT,								_("Select &Next Card\tPgDn"),		_("Selects the next card in the list"));
		menuCard->AppendSeparator();
		menuCard->Append(ID_CARD_ADD,		_("TOOL_CARD_ADD"),		_("&Add Card\tCtrl++"),				_("Add a new, blank, card to this set"));
		menuCard->Append(ID_CARD_ADD_MULT,	_("TOOL_CARD_ADD_M"),	_("Add &Multiple Cards..."),		_("Add multiple cards to the set"));
																	// NOTE: space after "Del" prevents wx from making del an accellerator
																	// otherwise we delete a card when delete is pressed inside the editor
		menuCard->Append(ID_CARD_REMOVE,	_("TOOL_CARD_DEL"),		_("&Remove Select Card\tDel "),		_("Delete the selected card from this set"));
		menuCard->AppendSeparator();
		IconMenu* menuRotate = new IconMenu();
			menuRotate->Append(ID_CARD_ROTATE_0,		_("TOOL_CARD_ROTATE_0"),	_("&Normal"),							_("Display the card with the right side up"),										wxITEM_CHECK);
			menuRotate->Append(ID_CARD_ROTATE_270,		_("TOOL_CARD_ROTATE_270"),	_("Rotated 90° &Clockwise"),			_("Display the card rotated clockwise"),											wxITEM_CHECK);
			menuRotate->Append(ID_CARD_ROTATE_90,		_("TOOL_CARD_ROTATE_90"),	_("Rotated 90° C&ounter Clockwise"),	_("Display the card rotated counter-clockwise (anti-clockwise for the British)"),	wxITEM_CHECK);
			menuRotate->Append(ID_CARD_ROTATE_180,		_("TOOL_CARD_ROTATE_180"),	_("Rotated 180°, &Up Side Down"),		_("Display the card up side down"),													wxITEM_CHECK);
		menuCard->Append(wxID_ANY,			_("TOOL_CARD_ROTATE"),	_("&Orientation"),					_("Orientation of the card display"),		wxITEM_NORMAL, menuRotate);
		menuCard->AppendSeparator();
		// This probably belongs in the window menu, but there we can't remove the separator once it is added
		menuCard->Append(ID_SELECT_COLUMNS,							_("C&ard List Columns..."),			_("Select what columns should be shown and in what order."));
	mb->Insert(2, menuCard,   _("&Cards"));
	
	IconMenu* menuFormat = new IconMenu();
		menuFormat->Append(ID_FORMAT_BOLD,		_("TOOL_BOLD"),		_("Bold\tCtrl+B"),					_("Makes the selected text bold"),			wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_ITALIC,	_("TOOL_ITALIC"),	_("Italic\tCtrl+I"),				_("Makes the selected text italic"),		wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_SYMBOL,	_("TOOL_SYMBOL"),	_("Symbols\tCtrl+M"),				_("Draws the selected text with symbols"),	wxITEM_CHECK);
	mb->Insert(3, menuFormat, _("&Format"));
}

void CardsPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->DeleteTool(ID_FORMAT_BOLD);
	tb->DeleteTool(ID_FORMAT_ITALIC);
	tb->DeleteTool(ID_FORMAT_SYMBOL);
	tb->DeleteTool(ID_CARD_ADD);
	tb->DeleteTool(ID_CARD_REMOVE);
	tb->DeleteTool(ID_CARD_ROTATE);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(10); // delete separator
	tb->DeleteToolByPos(10); // delete separator
	// Menus
	delete mb->Remove(3);
	delete mb->Remove(2);
}

void CardsPanel::onUpdateUI(wxUpdateUIEvent& e) {
	switch (e.GetId()) {
		case ID_CARD_PREV:       e.Enable(card_list->canSelectPrevious());	break;
		case ID_CARD_NEXT:       e.Enable(card_list->canSelectNext());		break;
/*		case ID_CARD_ROTATE_0:   e.Check(editor->rotation.angle == 0);		break;
		case ID_CARD_ROTATE_90:  e.Check(editor->rotation.angle == 90);		break;
		case ID_CARD_ROTATE_180: e.Check(editor->rotation.angle == 180);	break;
		case ID_CARD_ROTATE_270: e.Check(editor->rotation.angle == 270);	break;
		case ID_CARD_REMOVE:     e.Enable(set->cards.size() > 0);			break;
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: {
			if (focusedControl() == idEditor) {
				e.Enable(editor->canFormat(e.id));
				e.Check (editor->hasFormat(e.id));
			} else {
				e.Enable(false);
				e.Check(false);
			}
			break;
		}*/
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
		case ID_CARD_ROTATE:
			set->actions.add(new RemoveCardAction(*set, card_list->getCard()));
			break;
/*		case idCardRotate {
			StyleSettings& ss = settings.styleSettingsFor(*editor->style);
			ss.cardAngle = (ss.cardAngle + 90) % 360;
			onRenderSettingsChange();
		}
		case idCardRotate0 {
			StyleSettings& ss = settings.styleSettingsFor(*editor->style);
			ss.cardAngle = 0;
			onRenderSettingsChange();
		}
		case idCardRotate90 {
			StyleSettings& ss = settings.styleSettingsFor(*editor->style);
			ss.cardAngle = 90;
			onRenderSettingsChange();
		}
		case idCardRotate180 {
			StyleSettings& ss = settings.styleSettingsFor(*editor->style);
			ss.cardAngle = 180;
			onRenderSettingsChange();
		}
		case idCardRotate270 {
			StyleSettings& ss = settings.styleSettingsFor(*editor->style);
			ss.cardAngle = 270;
			onRenderSettingsChange();
		}
		case idSelectColumns {
			cardList->selectColumns();
		}
		case idFormatBold, idFormatItalic, idFormatSymbol, idFormatNoAuto {
			if (focusedControl() == idEditor) {
				editor->doFormat(id);
			}
		}*/
	}
}

// ----------------------------------------------------------------------------- : Actions

bool CardsPanel::wantsToHandle(const Action&) const {
	return false;
}

void CardsPanel::onAction(const Action& action) {
	// TODO
}

void CardsPanel::onRenderSettingsChange() {
}

// ----------------------------------------------------------------------------- : Clipboard
// ----------------------------------------------------------------------------- : Searching

// ----------------------------------------------------------------------------- : Selection

CardP CardsPanel::selectedCard() const {
	return card_list->getCard();
}
void CardsPanel::selectCard(const CardP& card) {
	card_list->setCard(card);
//	editor->setCard(card);
}