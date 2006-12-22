//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/cards_panel.hpp>
#include <gui/control/image_card_list.hpp>
#include <gui/control/card_editor.hpp>
#include <gui/control/text_ctrl.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/action/set.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <wx/splitter.h>

// ----------------------------------------------------------------------------- : CardsPanel

CardsPanel::CardsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id, false)
{
	// init controls
	wxPanel* notesP;
	wxSplitterWindow* splitter;
	editor    = new CardEditor(this, ID_EDITOR);
	splitter  = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	card_list = new ImageCardList(splitter, ID_CARD_LIST);
	notesP    = new Panel(splitter, wxID_ANY);
	notes     = new TextCtrl(notesP, ID_NOTES);
	// init sizer for notes panel
	wxSizer* sn = new wxBoxSizer(wxVERTICAL);
	sn->Add(new wxStaticText(notesP, wxID_ANY, _LABEL_("card notes")), 0, wxEXPAND, 2);
	sn->Add(notes, 1, wxEXPAND | wxTOP, 2);
	notesP->SetSizer(sn);
	// init splitter
	splitter->SetMinimumPaneSize(14);
	splitter->SetSashGravity(1.0);
	splitter->SplitHorizontally(card_list, notesP, -40);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(editor,   0, wxRIGHT, 2);
	s->Add(splitter, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
}

CardsPanel::~CardsPanel() {
//	settings.card_notes_height = splitter->GetSashPosition();
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
	tb->AddTool(ID_FORMAT_BOLD,		_(""), Bitmap(_("TOOL_BOLD")),		wxNullBitmap, wxITEM_CHECK, _TOOL_("bold"));
	tb->AddTool(ID_FORMAT_ITALIC,	_(""), Bitmap(_("TOOL_ITALIC")),	wxNullBitmap, wxITEM_CHECK, _TOOL_("italic"));
	tb->AddTool(ID_FORMAT_SYMBOL,	_(""), Bitmap(_("TOOL_SYMBOL")),	wxNullBitmap, wxITEM_CHECK, _TOOL_("symbols"));
	tb->AddTool(ID_FORMAT_REMINDER,	_(""), Bitmap(_("TOOL_REMINDER")),	wxNullBitmap, wxITEM_CHECK, _TOOL_("reminder text"));
	tb->AddSeparator();
	tb->AddTool(ID_CARD_ADD,		_(""), Bitmap(_("TOOL_CARD_ADD")),	wxNullBitmap, wxITEM_NORMAL,_TOOL_("add card"));
	tb->AddTool(ID_CARD_REMOVE,		_(""), Bitmap(_("TOOL_CARD_DEl")),	wxNullBitmap, wxITEM_NORMAL,_TOOL_("remove card"));
	tb->AddSeparator();
	tb->AddTool(ID_CARD_ROTATE,		_(""), Bitmap(_("TOOL_CARD_ROTATE")),wxNullBitmap,wxITEM_NORMAL,_TOOL_("rotate card"));
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
		menuFormat->Append(ID_FORMAT_REMINDER,	_("TOOL_REMINDER"),	_("Reminder Text\tCtrl+R"),			_("Show reminder text for the selected keyword"),	wxITEM_CHECK);
	mb->Insert(3, menuFormat, _("&Format"));
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
	tb->DeleteToolByPos(10); // delete separator
	tb->DeleteToolByPos(10); // delete separator
	// Menus
	delete mb->Remove(3);
	delete mb->Remove(2);
}

void CardsPanel::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_CARD_PREV:       ev.Enable(card_list->canSelectPrevious());	break;
		case ID_CARD_NEXT:       ev.Enable(card_list->canSelectNext());		break;
		case ID_CARD_ROTATE_0: case ID_CARD_ROTATE_90: case ID_CARD_ROTATE_180: case ID_CARD_ROTATE_270: {
			StyleSheetSettings& ss = settings.stylesheetSettingsFor(*set->stylesheetFor(card_list->getCard()));
			int a = ev.GetId() == ID_CARD_ROTATE_0   ? 0
			      : ev.GetId() == ID_CARD_ROTATE_90  ? 90
			      : ev.GetId() == ID_CARD_ROTATE_180 ? 180
			      :                                    270;
			ev.Check(ss.card_angle() == a);
			break;
		}
		case ID_CARD_REMOVE:     ev.Enable(set->cards.size() > 0);			break;
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
			set->actions.add(new RemoveCardAction(*set, card_list->getCard()));
			break;
		case ID_CARD_ROTATE:
		case ID_CARD_ROTATE_0: case ID_CARD_ROTATE_90: case ID_CARD_ROTATE_180: case ID_CARD_ROTATE_270: {
			StyleSheetSettings& ss = settings.stylesheetSettingsFor(*set->stylesheetFor(card_list->getCard()));
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
	}
}

// ----------------------------------------------------------------------------- : Actions

bool CardsPanel::wantsToHandle(const Action&, bool undone) const {
	return false;
}

void CardsPanel::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, DisplayChangeAction) {
		// The style changed, maybe also the size of editor
		Layout();
		//if (current_panel) current_panel->Layout();
		//fixMinWindowSize();
	}
}

void CardsPanel::onRenderSettingsChange() {
	// TODO
}

// ----------------------------------------------------------------------------- : Clipboard

bool CardsPanel::canCut()   const { return focused_control(this) == ID_EDITOR ? editor->canCut()   :    card_list->canCut();   }
bool CardsPanel::canCopy()  const { return focused_control(this) == ID_EDITOR ? editor->canCopy()  :    card_list->canCopy();  }
bool CardsPanel::canPaste() const { return focused_control(this) == ID_EDITOR ? editor->canPaste() :    card_list->canPaste(); }
void CardsPanel::doCut()          { if    (focused_control(this) == ID_EDITOR)  editor->doCut();   else card_list->doCut();    }
void CardsPanel::doCopy()         { if    (focused_control(this) == ID_EDITOR)  editor->doCopy();  else card_list->doCopy();   }
void CardsPanel::doPaste()        { if    (focused_control(this) == ID_EDITOR)  editor->doPaste(); else card_list->doPaste();  }

// ----------------------------------------------------------------------------- : Searching

// ----------------------------------------------------------------------------- : Selection

CardP CardsPanel::selectedCard() const {
	return card_list->getCard();
}
void CardsPanel::selectCard(const CardP& card) {
	card_list->setCard(card);
	editor->setCard(card);
	notes->setValue(card ? &card->notes : nullptr);
	Layout();
}
