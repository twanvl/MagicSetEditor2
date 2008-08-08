//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CARD_SELECT_WINDOW
#define HEADER_GUI_CARD_SELECT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(ExportCardSelectionChoice);
class SelectCardList;

// ----------------------------------------------------------------------------- : ExportWindowBase

enum ExportCardSelectionType
{	EXPORT_SEL_ENTIRE_SET
,	EXPORT_SEL_SUBSET
,	EXPORT_SEL_CUSTOM
};

class ExportCardSelectionChoice : public IntrusivePtrBase<ExportCardSelectionChoice> {
  public:
	ExportCardSelectionChoice();
	ExportCardSelectionChoice(const Set& set);
	ExportCardSelectionChoice(const String& label, const vector<CardP>& cards);
	ExportCardSelectionChoice(const String& label, const vector<CardP>* cards);
	
	const String                  label;
	const ExportCardSelectionType type;
	const vector<CardP>*          the_cards; ///< The cards
	vector<CardP>                 own_cards; ///< Maybe we own the cards, in that case the_cards = &own_cards
};

typedef vector<ExportCardSelectionChoiceP> ExportCardSelectionChoices;

/// Base class for export windows, deals with card selection
class ExportWindowBase : public wxDialog {
  public:
	ExportWindowBase(Window* parent, const String& window_title,
	                 const SetP& set, const ExportCardSelectionChoices& cards_choices);
	/// Create the controls, return a sizer containing them
	wxSizer* Create();
	
	/// Get the selected cards
	const vector<CardP>& getSelection() const { return *cards; }
	
  protected:
	DECLARE_EVENT_TABLE();
	
	SetP                 set;     ///< Set to export
	const vector<CardP>* cards;   ///< Cards to export
	
  private:
	ExportCardSelectionChoices cards_choices; ///< Ways to select cards
	size_t active_choice;
	wxStaticText* card_count;
	wxButton*     select_cards;
	
	void onChangeSelectionChoice(wxCommandEvent&);
	void onSelectCards(wxCommandEvent&);
	void update();
};

// ----------------------------------------------------------------------------- : CardSelectWindow

/// A window for selecting a subset of the cards from a set.
/** this is used when printing or exporting
 */
class CardSelectWindow : public wxDialog {
  public:
	CardSelectWindow(Window* parent, const SetP& set, const String& label, const String& title, bool sizer=true);
	
	/// Is the given card selected?
	bool isSelected(const CardP& card) const;
	/// Get a list of all selected cards
	void getSelection(vector<CardP>& out) const;
	/// Change which cards are selected
	void setSelection(const vector<CardP>& cards);
	
  protected:
	DECLARE_EVENT_TABLE();
	
	SelectCardList* list;
	SetP            set;
	wxButton*       sel_all, *sel_none;
	
	void onSelectAll (wxCommandEvent&);
	void onSelectNone(wxCommandEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
