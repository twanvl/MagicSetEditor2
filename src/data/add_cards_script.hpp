//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/scriptable.hpp>

class Set;
DECLARE_POINTER_TYPE(Card);

// ----------------------------------------------------------------------------- : AddCardsScript

/// A script to add one or more cards to a set
class AddCardsScript : public IntrusivePtrBase<AddCardsScript> {
public:
  String           name;
  String           description;
  Scriptable<bool> enabled;
  OptionalScript   script;
  
  /// Perform the script; return the cards (if any)
  void perform(Set& set, vector<CardP>& out);
  /// Perform the script; add cards to the set
  void perform(Set& set);
  
  DECLARE_REFLECTION();
};


