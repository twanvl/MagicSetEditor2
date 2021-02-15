//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_list.hpp>
#include <set>

// ----------------------------------------------------------------------------- : SelectCardList

/// A card list with check boxes
class SelectCardList : public CardListBase {
public:
  SelectCardList(Window* parent, int id, long additional_style = 0);
  ~SelectCardList();
  /// Select all cards
  void selectAll();
  /// Deselect all cards
  void selectNone();
  /// Is the given card selected?
  bool isSelected(const CardP& card) const;
  /// Get a list of all selected cards
  void getSelection(vector<CardP>& out) const;
  /// Change which cards are selected
  void setSelection(const vector<CardP>& cards);
  
protected:
  int OnGetItemImage(long pos) const override;
  void onChangeSet() override;
private:
  DECLARE_EVENT_TABLE();
  
  std::set<CardP> selected; ///< which cards are selected?

  void toggle(const CardP& card);
  void toggleSelected(bool select);
  
  void onKeyDown(wxKeyEvent&);
  void onLeftDown(wxMouseEvent&);
};


