//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/keywords_panel.hpp>
#include <data/keyword.hpp>
#include <wx/listctrl.h>

// ----------------------------------------------------------------------------- : KeywordsList

/// A control that lists the keywords in a set or game
class KeywordList : public wxListView {
  public:
	KeywordList(Window* parent, int id);
	
	/// Set the list of keywords to show
	void setData(vector<KeywordP>& dat);
	
	bool canSelectPrevious() const;
	bool canSelectNext() const;
	void selectPrevious();
	void selectNext();
};

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
