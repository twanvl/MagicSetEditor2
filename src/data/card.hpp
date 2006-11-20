//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_CARD
#define HEADER_DATA_CARD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>

class Game;
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Value);
DECLARE_POINTER_TYPE(StyleSheet);

// ----------------------------------------------------------------------------- : Card

/// A card from a card Set
class Card {
  public:
	/// Default constructor, uses game_for_new_cards to make the game
	Card();
	/// Creates a card using the given game
	Card(const Game& game);
	
	/// The values on the fields of the card.
	/** The indices should correspond to the card_fields in the Game */
	IndexMap<FieldP, ValueP> data;
	/// Notes for this card
	String notes;
	/// Alternative style to use for this card
	/** Optional; if not set use the card style from the set */
	StyleSheetP stylesheet;
	
	/// Get the identification of this card, an identification is something like a name, title, etc.
	/** May return "" */
	String identification() const;
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
