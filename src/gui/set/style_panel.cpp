//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/style_panel.hpp>
#include <gui/control/package_list.hpp>
#include <data/game.hpp>

// ----------------------------------------------------------------------------- : StylePanel

StylePanel::StylePanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	PackageList* list = new PackageList(this, wxID_ANY);
	list->showData<Game>();
	
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(list, 1, wxEXPAND);
	SetSizer(s);
}
