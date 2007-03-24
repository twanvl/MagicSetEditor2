//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/keywords_panel.hpp>
#include <gui/control/keyword_list.hpp>
#include <gui/control/text_ctrl.hpp>
#include <data/keyword.hpp>
#include <wx/listctrl.h>
#include <wx/splitter.h>

// ----------------------------------------------------------------------------- : KeywordsPanel

KeywordsPanel::KeywordsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	Panel* panel;
	splitter  = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	list      = new KeywordList(splitter, wxID_ANY);
	panel     = new Panel(splitter, wxID_ANY);
	keyword   = new TextCtrl(panel, wxID_ANY);
	match     = new TextCtrl(panel, wxID_ANY);
	reminder  = new TextCtrl(panel, wxID_ANY);
	rules     = new TextCtrl(panel, wxID_ANY);
	// init sizer for panel
	wxSizer* sp = new wxBoxSizer(wxVERTICAL);
		sp->Add(new wxStaticText(panel, wxID_ANY, _("Keyword:")), 0, wxALL, 6);
		sp->Add(keyword, 0, wxEXPAND | wxALL & ~wxTOP, 6);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, panel, _("Match"));
			s2->Add(new wxStaticText(panel, wxID_ANY, _("Keyword format:")), 0, wxALL, 6);
			s2->Add(match, 0, wxEXPAND | wxALL & ~wxTOP, 6);
			s2->Add(new wxStaticText(panel, wxID_ANY, _("Parameters:")), 0, wxALL, 6);
		sp->Add(s2, 0, wxEXPAND | wxALL, 6);
		sp->Add(new wxStaticText(panel, wxID_ANY, _("Reminder:")), 0, wxALL, 6);
		sp->Add(reminder, 0, wxEXPAND | wxALL & ~wxTOP, 6);
		sp->Add(new wxStaticText(panel, wxID_ANY, _("Rules:")), 0, wxALL, 6);
		sp->Add(rules, 0, wxEXPAND | wxALL & ~wxTOP, 6);
	panel->SetSizer(sp);
	// init splitter
	splitter->SetMinimumPaneSize(100);
	splitter->SetSashGravity(0.5);
	splitter->SplitVertically(list, panel, -200);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(splitter, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
	
	//s->Add(new wxStaticText(this, wxID_ANY, _("Sorry, no keywords for now"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTER), 1, wxALIGN_CENTER); // TODO: Remove
	/*	wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(list_active,   1, wxEXPAND);
			s2->Add(list_inactive, 1, wxEXPAND);*/
}

void KeywordsPanel::onChangeSet() {
	list->setSet(set);
}
