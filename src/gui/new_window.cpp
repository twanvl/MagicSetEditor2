//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

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
	: wxDialog(parent, wxID_ANY, _("New set"), wxDefaultPosition, wxSize(530,320), wxDEFAULT_DIALOG_STYLE)
{
	wxBusyCursor wait;
	// init controls
	game_list       = new PackageList (this, ID_GAME_LIST);
	stylesheet_list = new PackageList (this, ID_STYLESHEET_LIST);
	game_text       = new wxStaticText(this, ID_GAME_LIST,       _LABEL_("game type"));
	stylesheet_text = new wxStaticText(this, ID_STYLESHEET_LIST, _LABEL_("style type"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(game_text,       0, wxALL,                     4);
		s->Add(game_list,       0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(stylesheet_text, 0, wxALL,                     4);
		s->Add(stylesheet_list, 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
	// Resize
	SetSize(630,-1);
	Layout();
	GetSizer()->SetSizeHints(this);
	SetSize(630,-1);
	// init lists
	game_list->showData<Game>();
	try {
	game_list->select(settings.default_game);
	} catch (FileNotFoundError e) {
	handle_error(e);
	}
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void NewSetWindow::onGameSelect(wxCommandEvent&) {
	wxBusyCursor wait;
	GameP game = game_list->getSelection<Game>();
	settings.default_game = game->name();
	GameSettings& gs = settings.gameSettingsFor(*game);
	stylesheet_list->showData<StyleSheet>(game->name() + _("-*"));
	stylesheet_list->select(gs.default_stylesheet);
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
	// resize (yuck)
	SetSize(630,-1);
	Layout();
	GetSizer()->SetSizeHints(this);
	Layout();
	GetSizer()->SetSizeHints(this);
	SetSize(630,-1);
}

void NewSetWindow::onStyleSheetSelect(wxCommandEvent&) {
	// store this as default selection
	GameP       game       = game_list      ->getSelection<Game>();
	StyleSheetP stylesheet = stylesheet_list->getSelection<StyleSheet>();
	GameSettings& gs = settings.gameSettingsFor(*game);
	gs.default_stylesheet = stylesheet->name();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}
void NewSetWindow::onStyleSheetActivate(wxCommandEvent&) {
	done();
}

void NewSetWindow::OnOK(wxCommandEvent&) {
	done();
}

void NewSetWindow::done() {
	StyleSheetP stylesheet = stylesheet_list->getSelection<StyleSheet>();
	set = new_intrusive1<Set>(stylesheet);
	set->validate();
	EndModal(wxID_OK);
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

BEGIN_EVENT_TABLE(NewSetWindow, wxDialog)
	EVT_GALLERY_SELECT  (ID_GAME_LIST,       NewSetWindow::onGameSelect)
	EVT_GALLERY_SELECT  (ID_STYLESHEET_LIST, NewSetWindow::onStyleSheetSelect)
	EVT_GALLERY_ACTIVATE(ID_STYLESHEET_LIST, NewSetWindow::onStyleSheetActivate)
	EVT_BUTTON          (wxID_OK,            NewSetWindow::OnOK)
	EVT_UPDATE_UI       (wxID_ANY,           NewSetWindow::onUpdateUI)
END_EVENT_TABLE  ()
