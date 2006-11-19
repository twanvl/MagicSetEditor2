//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_FILTERED_CARD_LIST
#define HEADER_GUI_CONTROL_FILTERED_CARD_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_list.hpp>

DECLARE_POINTER_TYPE(CardListFilter);

// ----------------------------------------------------------------------------- : CardListFilter

/// A filter function to determine which items are shown in a card list
class CardListFilter {
  public:
	virtual ~CardListFilter() {}
	/// Should a card be shown in the list?
	virtual bool keep(const CardP& card) = 0;
};

// ----------------------------------------------------------------------------- : FilteredCardList

/// A card list that lists a subset of the cards in the set
class FilteredCardList : public CardListBase {
  public:
	FilteredCardList(Window* parent, int id, int style = 0);
	
	/// Change the filter to use
	void setFilter(const CardListFilterP& filter_);
	
  protected:
	/// Get only the subset of the cards
	virtual const vector<CardP>& getCards() const;
	/// Rebuild the filtered card list
	virtual void onRebuild();
//	/// Don't reorder
//	virtual void onDrag(wxMouseEvent& e);

  private:	
	CardListFilterP filter;	///< Filter with which this.cards is made
	vector<CardP>   cards;	///< The cards that are shown
};


// ----------------------------------------------------------------------------- : EOF
#endif
