//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_list_column_select.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <util/window_id.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(CardListColumnSelectDialog::ColumnSettingsF);

// ----------------------------------------------------------------------------- : CardListColumnSelectDialog

CardListColumnSelectDialog::CardListColumnSelectDialog(Window* parent, const GameP& game)
	: wxDialog(parent, wxID_ANY, _TITLE_("select columns"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	, game(game)
{
	// Create controls
	list = new wxCheckListBox(this, wxID_ANY);
	// Create sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("select columns")), 0, wxALL, 8);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("columns")       ), 0, wxALL & ~wxBOTTOM, 8);
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			s2->Add(list, 1, wxEXPAND | wxLEFT | wxRIGHT, 4);
			wxSizer* s3 = new wxBoxSizer(wxVERTICAL);
				s3->Add(new wxButton(this, ID_MOVE_UP,   _BUTTON_("move up")),   0, wxEXPAND,         2);
				s3->Add(new wxButton(this, ID_MOVE_DOWN, _BUTTON_("move down")), 0, wxEXPAND | wxTOP, 2);
				s3->Add(new wxButton(this, ID_SHOW,      _BUTTON_("show")),      0, wxEXPAND | wxTOP, 2);
				s3->Add(new wxButton(this, ID_HIDE,      _BUTTON_("hide")),      0, wxEXPAND | wxTOP, 2);
			s2->Add(s3,   0, wxEXPAND | wxALL & ~wxTOP,  4);
		s->Add(s2                                                                           , 1, wxEXPAND | wxALL, 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL)                                           , 0, wxEXPAND | wxALL, 8);
	s->SetSizeHints(this);
	SetSizer(s);
	// Set default size
	SetSize(350, 450);
	// Initialize order list
	initColumns();
	initList();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

struct SortByPosition {
	SortByPosition(const Game& game) : game(game) {}
	const Game& game;
	bool operator() (const CardListColumnSelectDialog::ColumnSettingsF& a, const CardListColumnSelectDialog::ColumnSettingsF& b){
		return a.settings.position < b.settings.position;
	}
};

void CardListColumnSelectDialog::initColumns() {
	// order is a list of all columns that may be shown
	FOR_EACH(f, game->card_fields) {
		if (f->card_list_allow) {
			columns.push_back(ColumnSettingsF(f, settings.columnSettingsFor(*game, *f)));
		}
	}
	// sorted by position
	sort(columns.begin(), columns.end(), SortByPosition(*game));
	// force unique position
	int min = 0;
	FOR_EACH(c, columns) {
		if (c.settings.position < min) c.settings.position = min;
		min = c.settings.position + 1;
	}
}

void CardListColumnSelectDialog::initList() {
	// Init items
	Color window_color = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	FOR_EACH(c, columns) {
		list->Append(tr(*game, c.field->card_list_name, capitalize(c.field->card_list_name)));
		// check
		int i = list->GetCount() - 1;
		list->Check(i, c.settings.visible);
		#ifdef __WXMSW__
			// fix the background color
			list->GetItem(i)->SetBackgroundColour(window_color);
		#endif
	}
}

void CardListColumnSelectDialog::refreshItem(int i) {
	list->Check    (i, columns[i].settings.visible);
	list->SetString(i, tr(*game, columns[i].field->card_list_name, capitalize(columns[i].field->card_list_name)) );
}

// ----------------------------------------------------------------------------- : Events

void CardListColumnSelectDialog::onSelect(wxCommandEvent& ev) {
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void CardListColumnSelectDialog::onCheck(wxCommandEvent& ev) {
	int i = ev.GetSelection();
	columns[i].settings.visible = list->IsChecked(i);
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void CardListColumnSelectDialog::onMove(wxCommandEvent& ev) {
	int i = list->GetSelection();
	int delta = ev.GetId() == ID_MOVE_UP ? -1 : 1;
	if (i == wxNOT_FOUND || i + delta < 0 || i + delta >= (int)columns.size()) return;
	list->SetSelection(i + delta);
	// swap the columns and positions
	swap(columns[i],                   columns[i + delta]);
	swap(columns[i].settings.position, columns[i + delta].settings.position);
	refreshItem(i);
	refreshItem(i + delta);
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void CardListColumnSelectDialog::onShowHide(wxCommandEvent& ev) {
	int i = list->GetSelection();
	if (i == wxNOT_FOUND) return;
	columns[i].settings.visible = ev.GetId() == ID_SHOW;
	refreshItem(i);
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void CardListColumnSelectDialog::onOk(wxCommandEvent&) {
	// store column settings
	FOR_EACH(c, columns) {
		settings.columnSettingsFor(*game, *c.field) = c.settings;
	}
	// close dialog
	EndModal(wxID_OK);
	
}

void CardListColumnSelectDialog::onUpdateUI(wxUpdateUIEvent& ev) {
	int i = list->GetSelection();
	switch (ev.GetId()) {
		case ID_MOVE_UP:	ev.Enable(i != wxNOT_FOUND && i - 1 >= 0);						break;
		case ID_MOVE_DOWN:	ev.Enable(i != wxNOT_FOUND && i + 1 < (int)columns.size());		break;
		case ID_SHOW:		ev.Enable(i != wxNOT_FOUND && !columns[i].settings.visible);	break;
		case ID_HIDE:		ev.Enable(i != wxNOT_FOUND && columns[i].settings.visible);		break;
	}
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(CardListColumnSelectDialog, wxDialog)
	EVT_LISTBOX        (wxID_ANY,     CardListColumnSelectDialog::onSelect)
	EVT_CHECKLISTBOX   (wxID_ANY,     CardListColumnSelectDialog::onCheck)
	EVT_BUTTON         (ID_MOVE_UP,   CardListColumnSelectDialog::onMove)
	EVT_BUTTON         (ID_MOVE_DOWN, CardListColumnSelectDialog::onMove)
	EVT_BUTTON         (ID_SHOW,      CardListColumnSelectDialog::onShowHide)
	EVT_BUTTON         (ID_HIDE,      CardListColumnSelectDialog::onShowHide)
	EVT_BUTTON         (wxID_OK,      CardListColumnSelectDialog::onOk)
	EVT_UPDATE_UI      (wxID_ANY,     CardListColumnSelectDialog::onUpdateUI)
END_EVENT_TABLE  ()
