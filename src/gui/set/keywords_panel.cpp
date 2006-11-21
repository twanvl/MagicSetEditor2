//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/keywords_panel.hpp>

// ----------------------------------------------------------------------------- : KeywordsList

// ----------------------------------------------------------------------------- : KeywordsPanel

KeywordsPanel::KeywordsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	/*wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(list_active,   1, wxEXPAND);
			s2->Add(list_inactive, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);*/
}
