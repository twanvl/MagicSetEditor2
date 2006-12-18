//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/card_select_window.hpp>
#include <gui/control/select_card_list.hpp>
#include <util/window_id.hpp>

// ----------------------------------------------------------------------------- : CardSelectWindow

CardSelectWindow::CardSelectWindow(Window* parent, const SetP& set, const String& label)
	: wxDialog(parent, wxID_ANY, _TITLE_("select cards"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, set(set)
{
	// init controls
	list = new SelectCardList(this, wxID_ANY);
	list->setSet(set);
	wxButton* sel_all  = new wxButton(this, ID_SELECT_ALL,  _BUTTON_("select all"));
	wxButton* sel_none = new wxButton(this, ID_SELECT_NONE, _BUTTON_("select none"));
	// init sizers
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALL & ~wxBOTTOM, 8);
		s->Add(list, 1, wxEXPAND | wxALL, 8);
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			s2->Add(sel_all,  0, wxEXPAND | wxRIGHT, 8);
			s2->Add(sel_none, 0, wxEXPAND | wxRIGHT, 8);
			s2->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxEXPAND, 8);
		s->Add(s2, 0, wxEXPAND | wxALL & ~wxTOP, 8);
	s->SetSizeHints(this);
	SetSizer(s);
	SetSize(500,500);
}

bool CardSelectWindow::isSelected(const CardP& card) const {
	return list->isSelected(card);
}

void CardSelectWindow::onSelectAll(wxCommandEvent&) {
	list->selectAll();
}
void CardSelectWindow::onSelectNone(wxCommandEvent&) {
	list->selectNone();
}

BEGIN_EVENT_TABLE(CardSelectWindow,wxDialog)
	EVT_BUTTON       (ID_SELECT_ALL,  CardSelectWindow::onSelectAll)
	EVT_BUTTON       (ID_SELECT_NONE, CardSelectWindow::onSelectNone)
END_EVENT_TABLE  ()
