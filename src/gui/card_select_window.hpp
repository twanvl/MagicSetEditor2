//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CARD_SELECT_WINDOW
#define HEADER_GUI_CARD_SELECT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Card);
class SelectCardList;

// ----------------------------------------------------------------------------- : CardSelectWindow

/// A window for selecting a subset of the cards from a set.
/** this is used when printing or exporting
 */
class CardSelectWindow : public wxDialog {
  public:
	CardSelectWindow(Window* parent, const SetP& set, const String& label);
	
	/// Is the given card selected?
	bool isSelected(const CardP& card) const;
	
  protected:
	DECLARE_EVENT_TABLE();
	
	SelectCardList* list;
	SetP            set;
	
	void onSelectAll (wxCommandEvent&);
	void onSelectNone(wxCommandEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
