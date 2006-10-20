//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	: wxDialog(parent, wxID_ANY, _("New set"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	// init controls
	game_list       = new PackageList (this, ID_GAME_LIST);
	stylesheet_list = new PackageList (this, ID_STYLESHEET_LIST);
	game_text       = new wxStaticText(this, wxID_ANY, _("&Game type:"));
	stylesheet_text = new wxStaticText(this, wxID_ANY, _("&Card style:"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(game_text,       0, wxALL,                     4);
		s->Add(game_list,       1, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(stylesheet_text, 0, wxALL,                     4);
		s->Add(stylesheet_list, 1, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
	// disable stuff
	ok_button = FindWindow(wxID_OK);
	assert(ok_button);
	enableStyle(false);
	// force refresh of gameList, otherwise a grey background shows (win XP)
	SetSize(wxSize(530,320));
	// init lists
	game_list->showData<Game>();
	game_list->select(settings.default_game);
}

void NewSetWindow::enableStyle(bool e) {
	stylesheet_list->Enable(e);
	stylesheet_text->Enable(e);
	if (!e) enableOk(e);
}
void NewSetWindow::enableOk(bool e) {
	ok_button->Enable(e);
}

void NewSetWindow::onGameSelect(wxCommandEvent&) {
	GameP game = game_list->getSelection<Game>();
	settings.default_game = game->name();
	GameSettings& gs = settings.gameSettingsFor(*game);
	stylesheet_list->showData<StyleSheet>(game->name() + _("-*"));
	stylesheet_list->select(gs.default_stylesheet);
	enableStyle(true);
}
/*void NewSetWindow::onGameDeselect(wxCommandEvent&) {
	stylesheet_list->clear();
	enableStyle(false);
}
*/
void NewSetWindow::onStyleSheetSelect(wxCommandEvent&) {
	enableOk(true);
	// store this as default selection
	GameP       game       = game_list      ->getSelection<Game>();
	StyleSheetP stylesheet = stylesheet_list->getSelection<StyleSheet>();
	GameSettings& gs = settings.gameSettingsFor(*game);
	gs.default_stylesheet = stylesheet->name();
}
/*void NewSetWindow::onStyleSheetDeselect(wxCommandEvent&) {
	enableOk(false);
}
*/
void NewSetWindow::onStyleSheetActivate(wxCommandEvent&) {
	done();
}

void NewSetWindow::OnOK(wxCommandEvent&) {
	done();
}

void NewSetWindow::done() {
	StyleSheetP stylesheet = stylesheet_list->getSelection<StyleSheet>();
	set = new_shared1<Set>(stylesheet);
	set->cards.push_back(new_shared1<Card>(*set->game));
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(NewSetWindow, wxDialog)
	EVT_GALLERY_SELECT  (ID_GAME_LIST,  NewSetWindow::onGameSelect)
//	EVT_LIST_ITEM_DESELECTED (ID_GAME_LIST,  NewSetWindow::onGameDeselect)
	EVT_GALLERY_SELECT   (ID_STYLESHEET_LIST, NewSetWindow::onStyleSheetSelect)
//	EVT_LIST_ITEM_DESELECTED (ID_STYLESHEET_LIST, NewSetWindow::onStyleDeselect)
	EVT_GALLERY_ACTIVATE  (ID_STYLESHEET_LIST, NewSetWindow::onStyleSheetActivate)
END_EVENT_TABLE  ()
