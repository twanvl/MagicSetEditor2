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
#include <util/io/package.hpp>

DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(Stylesheet);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Value);

// ----------------------------------------------------------------------------- : Set

/// A set of cards
class Set : public Packaged {
  public:
	/// Create a set, the set should be open()ed later
	Set();
	/// Create a set using the given game
	Set(const GameP& game);
  
	/// The game this set uses
	GameP game;
	/// The default stylesheet
	StylesheetP stylesheet;
	/// The values on the fields of the set
	/** The indices should correspond to the set_fields in the Game */
	IndexMap<FieldP, ValueP> data;
	/// The cards in the set
	vector<CardP> cards;
	/// Code to use for apprentice (Magic only)
	String apprentice_code;
	/// Actions performed on this set and the cards in it
	ActionStack actions;
	
  protected:
	String typeName() const;
		
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : SetView

/// A 'view' of a Set, is notified when the Set is updated
/** To listen to events, derived classes should override onAction(const Action&, bool undone)
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
	
	/// Called when another set is being viewed (using setSet)
	virtual void onChangeSet() {}
	/// Called when just before another set is being viewed (using setSet)
	virtual void onBeforeChangeSet() {}
};


// ----------------------------------------------------------------------------- : EOF
#endif
