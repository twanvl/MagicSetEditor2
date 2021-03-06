//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_list.hpp>
#include <data/filter.hpp>

typedef intrusive_ptr<Filter<Card>> CardListFilterP;

// ----------------------------------------------------------------------------- : FilteredCardList

/// A card list that lists a subset of the cards in the set
class FilteredCardList : public CardListBase {
public:
  FilteredCardList(Window* parent, int id, long additional_style = 0);
  
  /// Change the filter to use
  void setFilter(const CardListFilterP& filter);
  
protected:
  /// Get only the subset of the cards
  void getItems(vector<VoidP>& out) const override;
  
  void onChangeSet() override;
  
private:
  CardListFilterP filter;  ///< Filter with which this.cards is made
};


