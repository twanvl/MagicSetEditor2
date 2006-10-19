//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_SET
#define HEADER_DATA_ACTION_SET

/** @file data/action/set.hpp
 *
 *  Actions operating on Sets
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>

class Set;
DECLARE_POINTER_TYPE(Card);

// ----------------------------------------------------------------------------- : Add card

/// An Action the changes the card list of a set
class CardListAction : public Action {
  public:
	inline CardListAction(Set& set) : set(set) {}
	
  protected:
	Set& set; // the set owns this action, so the set will not be destroyed before this
};

/// Adding a new card to a set
class AddCardAction : public CardListAction {
  public:
	AddCardAction(Set& set);
	AddCardAction(Set& set, const CardP& card);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	CardP card; ///< The new card
};

// ----------------------------------------------------------------------------- : Remove card

/// Removing a card from a set
class RemoveCardAction : public CardListAction {
  public:
	RemoveCardAction(Set& set, const CardP& card);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	CardP  card;	///< The removed card
	size_t card_id;	///< Position of the card in the set
};

// ----------------------------------------------------------------------------- : Reorder cards

/// Change the position of a card in the card list by swapping two cards
class ReorderCardsAction : public CardListAction {
  public:
	ReorderCardsAction(Set& set, size_t card_id1, size_t card_id2);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	size_t card_id1, card_id2;	///< Positions of the two cards to swap
};

// ----------------------------------------------------------------------------- : EOF
#endif
