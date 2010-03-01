//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ADD_CARDS_SCRIPT
#define HEADER_DATA_ADD_CARDS_SCRIPT

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


// ----------------------------------------------------------------------------- : EOF
#endif
