//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
DECLARE_POINTER_TYPE(StyleSheet);

// ----------------------------------------------------------------------------- : Add card

/// An Action the changes the card list of a set
class CardListAction : public Action {
  public:
	inline CardListAction(Set& set) : set(set) {}
	
  protected:
	Set& set; // the set owns this action, so the set will not be destroyed before this
	          // therefore we don't need a smart pointer
};

/// Adding a new card to a set
class AddCardAction : public CardListAction {
  public:
	AddCardAction(Set& set);
	AddCardAction(Set& set, const CardP& card);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	const CardP card; ///< The new card
};

// ----------------------------------------------------------------------------- : Remove card

/// Removing a card from a set
class RemoveCardAction : public CardListAction {
  public:
	RemoveCardAction(Set& set, const CardP& card);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	const CardP  card;	///< The removed card
	const size_t card_id;	///< Position of the card in the set
};

// ----------------------------------------------------------------------------- : Reorder cards

/// Change the position of a card in the card list by swapping two cards
class ReorderCardsAction : public CardListAction {
  public:
	ReorderCardsAction(Set& set, size_t card_id1, size_t card_id2);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	const size_t card_id1, card_id2;	///< Positions of the two cards to swap
};

// ----------------------------------------------------------------------------- : Change stylesheet

/// An action that affects the rendering/display/look of a set or cards in the set
class DisplayChangeAction : public Action {
  public:
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
};

/// Changing the style of a a card
class ChangeCardStyleAction : public DisplayChangeAction {
  public:
	ChangeCardStyleAction(const CardP& card, const StyleSheetP& stylesheet)
		: card(card), stylesheet(stylesheet) {}
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	CardP       card;			///< The affected card
	StyleSheetP stylesheet;		///< Its new stylesheet
};

/// Changing the style of a set to that of a card
class ChangeSetStyleAction : public DisplayChangeAction {
  public:
	ChangeSetStyleAction(Set& set, const CardP& card)
		: set(set), card(card) {}
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	Set&        set;			///< The affected set
	CardP       card;			///< The card whos stylesheet is copied to the set
	StyleSheetP stylesheet;		///< The old stylesheet of the set
};

// ----------------------------------------------------------------------------- : EOF
#endif
