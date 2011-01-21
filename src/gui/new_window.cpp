//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/new_window.hpp>
#include <gui/control/gallery_list.hpp>
#include <gui/control/package_list.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>

// ----------------------------------------------------------------------------- : NewSetWindow

SetP new_set_window(Window* parent) {
	NewSetWindow wnd(parent);
	wnd.ShowModal();
	return wnd.set;
}

NewSetWindow::NewSetWindow(Window* parent)
	: wxDialog(parent, wxID_ANY, _TITLE_("new set"), wxDefaultPosition, wxSize(530,320), wxDEFAULT_DIALOG_STYLE)
{
	wxBusyCursor wait;
	// init controls
	game_list       = new PackageList (this, ID_GAME_LIST,       wxHORIZONTAL, false);
	stylesheet_list = new PackageList (this, ID_STYLESHEET_LIST, wxHORIZONTAL, false);
	wxStaticText* game_text       = new wxStaticText(this, ID_GAME_LIST,       _LABEL_("game type"));
	wxStaticText* stylesheet_text = new wxStaticText(this, ID_STYLESHEET_LIST, _LABEL_("style type"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(game_text,       0, wxALL,                     4);
		s->Add(game_list,       0, wxEXPAND | (wxALL & ~wxTOP), 4);
		s->Add(stylesheet_text, 0, wxALL,                     4);
		s->Add(stylesheet_list, 0, wxEXPAND | (wxALL & ~wxTOP), 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
	// Resize
	Layout();
	wxSize min_size = GetSizer()->GetMinSize() + GetSize() - GetClientSize();
	SetSize(630,min_size.y);
	// init lists
	game_list->showData<Game>();
	try {
		game_list->select(settings.default_game);
	} catch (FileNotFoundError e) {
		handle_error(e);
	}
	game_list->SetFocus();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void NewSetWindow::onGameSelect(wxCommandEvent&) {
	wxBusyCursor wait;
	GameP game = game_list->getSelection<Game>(false);
	//handle_pending_errors(); // errors are ignored until set window is shown
	settings.default_game = game->name();
	stylesheet_list->showData<StyleSheet>(game->name() + _("-*"));
	stylesheet_list->select(settings.gameSettingsFor(*game).default_stylesheet);
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
	// resize (yuck)
	Layout();
	wxSize min_size = GetSizer()->GetMinSize() + GetSize() - GetClientSize();
	SetSize(630,min_size.y);
}

void NewSetWindow::onStyleSheetSelect(wxCommandEvent&) {
	// store this as default selection
	GameP       game       = game_list      ->getSelection<Game>(false);
	StyleSheetP stylesheet = stylesheet_list->getSelection<StyleSheet>(false);
	//handle_pending_errors(); // errors are ignored until set window is shown
	settings.gameSettingsFor(*game).default_stylesheet = stylesheet->name();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}
void NewSetWindow::onStyleSheetActivate(wxCommandEvent&) {
	done();
}

void NewSetWindow::OnOK(wxCommandEvent&) {
	done();
}

void NewSetWindow::done() {
	try {
		if (!stylesheet_list->hasSelection()) return;
		StyleSheetP stylesheet = stylesheet_list->getSelection<StyleSheet>();
		set = intrusive(new Set(stylesheet));
		set->validate();
		EndModal(wxID_OK);
	} catch (const Error& e) {
		handle_error(e);
	}
}

void NewSetWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_STYLESHEET_LIST:
			ev.Enable(game_list->hasSelection());
			break;
		case wxID_OK:
			ev.Enable(stylesheet_list->hasSelection());
			break;
	}
}

void NewSetWindow::onIdle(wxIdleEvent& ev) {
	// Stuff that must be done in the main thread
	//handle_pending_errors(); // errors are ignored until set window is shown
}

BEGIN_EVENT_TABLE(NewSetWindow, wxDialog)
	EVT_GALLERY_SELECT  (ID_GAME_LIST,       NewSetWindow::onGameSelect)
	EVT_GALLERY_SELECT  (ID_STYLESHEET_LIST, NewSetWindow::onStyleSheetSelect)
	EVT_GALLERY_ACTIVATE(ID_STYLESHEET_LIST, NewSetWindow::onStyleSheetActivate)
	EVT_BUTTON          (wxID_OK,            NewSetWindow::OnOK)
	EVT_UPDATE_UI       (wxID_ANY,           NewSetWindow::onUpdateUI)
	EVT_IDLE            (                    NewSetWindow::onIdle)
END_EVENT_TABLE  ()

// ----------------------------------------------------------------------------- : SelectStyleSheetWindow


StyleSheetP select_stylesheet(const Game& game, const String& failed_name) {
	SelectStyleSheetWindow wnd(nullptr, game, failed_name);
	wnd.ShowModal();
	return wnd.stylesheet;
}

SelectStyleSheetWindow::SelectStyleSheetWindow(Window* parent, const Game& game, const String& failed_name)
	: wxDialog(parent, wxID_ANY, _TITLE_("select stylesheet"), wxDefaultPosition, wxSize(530,320), wxDEFAULT_DIALOG_STYLE)
	, game(game)
{
	wxBusyCursor wait;
	// init controls
	stylesheet_list = new PackageList (this, ID_STYLESHEET_LIST);
	wxStaticText* description     = new wxStaticText(this, ID_GAME_LIST,       _LABEL_1_("stylesheet not found", failed_name));
	wxStaticText* stylesheet_text = new wxStaticText(this, ID_STYLESHEET_LIST, _LABEL_("style type"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(description,     0, wxALL,                     4);
		s->Add(stylesheet_text, 0, wxALL,                     4);
		s->Add(stylesheet_list, 0, wxEXPAND | (wxALL & ~wxTOP), 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
	// init list
	stylesheet_list->showData<StyleSheet>(game.name() + _("-*"));
	stylesheet_list->select(settings.gameSettingsFor(game).default_stylesheet);
	// Resize
	Layout();
	wxSize min_size = GetSizer()->GetMinSize() + GetSize() - GetClientSize();
	SetSize(630,min_size.y);
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void SelectStyleSheetWindow::onStyleSheetSelect(wxCommandEvent&) {
	//handle_pending_errors(); // errors are ignored until set window is shown
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}
void SelectStyleSheetWindow::onStyleSheetActivate(wxCommandEvent&) {
	done();
}

void SelectStyleSheetWindow::OnOK(wxCommandEvent&) {
	done();
}

void SelectStyleSheetWindow::done() {
	try {
		stylesheet = stylesheet_list->getSelection<StyleSheet>();
		EndModal(wxID_OK);
	} catch (const Error& e) {
		handle_error(e);
		EndModal(wxID_CANCEL);
	}
}

void SelectStyleSheetWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case wxID_OK:
			ev.Enable(stylesheet_list->hasSelection());
			break;
	}
}

void SelectStyleSheetWindow::onIdle(wxIdleEvent& ev) {
	// Stuff that must be done in the main thread
	//handle_pending_errors(); // errors are ignored until set window is shown
}

BEGIN_EVENT_TABLE(SelectStyleSheetWindow, wxDialog)
	EVT_GALLERY_SELECT  (ID_STYLESHEET_LIST, SelectStyleSheetWindow::onStyleSheetSelect)
	EVT_GALLERY_ACTIVATE(ID_STYLESHEET_LIST, SelectStyleSheetWindow::onStyleSheetActivate)
	EVT_BUTTON          (wxID_OK,            SelectStyleSheetWindow::OnOK)
	EVT_UPDATE_UI       (wxID_ANY,           SelectStyleSheetWindow::onUpdateUI)
	EVT_IDLE            (                    SelectStyleSheetWindow::onIdle)
END_EVENT_TABLE  ()

