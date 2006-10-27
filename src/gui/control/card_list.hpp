//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_CARD_LIST
#define HEADER_GUI_CONTROL_CARD_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/set.hpp>
#include <wx/listctrl.h>

DECLARE_POINTER_TYPE(ChoiceStyle);
DECLARE_POINTER_TYPE(Field);

// ----------------------------------------------------------------------------- : Events

DECLARE_EVENT_TYPE(EVENT_CARD_SELECT, <not used>)
/// Handle CardSelectEvents
#define EVT_CARD_SELECT(id, handler)										\
	DECLARE_EVENT_TABLE_ENTRY(EVENT_CARD_SELECT, id, -1,					\
	 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)		\
	 (void (wxEvtHandler::*)(CardSelectEvent&)) (&handler), (wxObject*) NULL),

/// The event of selecting a card
struct CardSelectEvent : public wxCommandEvent {
	CardP card; ///< The selected card
	inline CardSelectEvent(const CardP& card)
		: wxCommandEvent(EVENT_CARD_SELECT), card(card)
	{}
};

// ----------------------------------------------------------------------------- : CardListBase

/// A list view of the cards in a set.
/*  This class allows the cards to be sorted, and has a _('currentCard'), the selected card
 *  when a card is selected, it raises a CardSelectEvent, that will propage to the parent window.
 *
 *  Note: (long) pos refers to position in the sorted_card_list,
 *        (size_t) index refers to the index in the actual card list (as returned by getCards).
 *
 *  This class is an abstract base class for card lists, derived classes must overload:
 *   - getCard(index)
 */
class CardListBase : public wxListView, public SetView {
  public:
	CardListBase(Window* parent, int id, int additional_style = 0);
	~CardListBase();
	
	// --------------------------------------------------- : Selection
	
	inline CardP getCard() const            { return selected_card; }
	inline void  setCard(const CardP& card) { selectCard(card, true, false); }
	
	/// Is there a previous card to select?
	bool canSelectPrevious() const;
	/// Is there a next card to select?
	bool canSelectNext() const;
	/// Move the selection to the previous card (if possible)
	void selectPrevious();
	/// Move the selection to the next card (if possible)
	void selectNext();
	
	// --------------------------------------------------- : Clipboard
	
	bool canCut()   const;
	bool canCopy()  const;
	bool canPaste() const;
	void doCut();
	void doCopy();
	void doPaste();
	
	// --------------------------------------------------- : Set actions
	
	virtual void onBeforeChangeSet();
	virtual void onChangeSet();
	virtual void onAction(const Action&, bool undone);
	
	// --------------------------------------------------- : The cards
  protected:
	/// What cards should be shown?
	virtual vector<CardP>& getCards() const;
	
	// --------------------------------------------------- : Item 'events'
	
	/// Get the text of an item in a specific column
	/** Overrides a function from wxListCtrl */
	String OnGetItemText (long pos, long col) const;
	/// Get the image of an item, by default no image is used
	/** Overrides a function from wxListCtrl */
	int    OnGetItemImage(long pos) const;
	/// Get the color for an item
	wxListItemAttr* OnGetItemAttr(long pos) const;
	
	// --------------------------------------------------- : Data
  private:
	CardP          selected_card;	 ///< The currently selected card, or -1 if no card is selected
	long           selected_card_pos;///< Position of the selected card in the sorted_card_list
	// display stuff
	ChoiceStyleP   color_style;       ///< Style (and field) to use for text color (optional)
	vector<FieldP> column_fields;     ///< The field to use for each column (by column index)
	// sorted list stuff
	vector<CardP>  sorted_card_list; ///< Sorted list of cards, can be considered a map: pos->card
	FieldP         sort_criterium;   ///< Field to sort by
	bool           sort_ascending;   ///< Sort order
	
	mutable wxListItemAttr item_attr; // for OnGetItemAttr
	
//	/// Get a card by position
//	void getCard(long pos);
	
	/// Select a card, send an event to the parent
	/** If focus then the card is also focused and selected in the actual control.
	 *  This should abviously not be done when the card is selected because it was selected (leading to a loop).
	 */
	void selectCard(const CardP& card, bool focus, bool event);
	/// Select a card at the specified position
	void selectCardPos(long pos, bool focus);
	/// Find the position for the selected_card
	void findSelectedCardPos();
	/// Actually select the card at selected_card_pos in the control
	void selectCurrentCard();
	
	/// Sorts the list by the current sorting criterium
	void sortList();
	struct CardComparer; // for comparing cards
	/// Rebuild the card list (clear all vectors and fill them again)
	void rebuild();
	/// Refresh the card list (resort, refresh and reselect current item)
	void refreshList();
	/// Find the field that determines the color, if any.
	/** Note: Uses only fields from the set's default style */
	ChoiceStyleP findColorStyle();
	
	/// Store the column sizes in the settings
	void storeColumns();
	/// Open a dialog for selecting columns to be shown
	void selectColumns();
	
	// --------------------------------------------------- : Window events
	DECLARE_EVENT_TABLE();
	
	void onColumnClick     (wxListEvent& ev);
	void onColumnRightClick(wxListEvent& ev);
	void onSelectColumns   (wxCommandEvent& ev);
	void onItemFocus       (wxListEvent& ev);
	void onChar            (wxKeyEvent& ev);
	void onDrag            (wxMouseEvent& ev);
};

// ----------------------------------------------------------------------------- : EOF
#endif
