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
	
	// --------------------------------------------------- : Selection
	
	inline KeywordP getKeyword() const            { return selected_keyword; }
	inline void     setKeyword(const KeywordP& kw) { /* TODO */ }
	
	bool canSelectPrevious() const;
	bool canSelectNext() const;
	void selectPrevious();
	void selectNext();
	
  protected:
	/// Get the text of an item in a specific column
	/** Overrides a function from wxListCtrl */
	virtual String OnGetItemText (long pos, long col) const;
  private:
	KeywordP selected_keyword;
	long     selected_keyword_pos;
};

// ----------------------------------------------------------------------------- : KeywordsPanel

KeywordsPanel::KeywordsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(new wxStaticText(this, wxID_ANY, _("Sorry, no keywords for now"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTER), 1, wxALIGN_CENTER); // TODO: Remove
	/*	wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(list_active,   1, wxEXPAND);
			s2->Add(list_inactive, 1, wxEXPAND);*/
	s->SetSizeHints(this);
	SetSizer(s);
}
