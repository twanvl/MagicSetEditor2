//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_SET
#define HEADER_DATA_SET

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/action_stack.hpp>

DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Game);

// ----------------------------------------------------------------------------- : Set

/// A set of cards
class Set {
  public:
	/// The game this set uses
	GameP game;
	/// The cards in the set
	vector<CardP> cards;
	/// Actions performed on this set and the cards in it
	ActionStack actions;
	
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : SetView

/// A 'view' of a Set, is notified when the Set is updated
/** To listen to events, derived classes should override onAction(const Action&)
 */
class SetView : public ActionListener {
  public:
	SetView();
	~SetView();
  
	/// Get the set that is currently being viewed
	inline SetP getSet() { return set; }
	/// Change the set that is being viewed
	void setSet(const SetP& set);
	
  protected:
	/// The set that is currently being viewed, should not be modified directly!
	SetP set;
	
	/// Called when another set is being viewn (using setSet)
	virtual void onChangeSet() {}
};


// ----------------------------------------------------------------------------- : EOF
#endif
