//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_CARD
#define HEADER_DATA_CARD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/error.hpp>
#include <data/field.hpp> // for Card::value

class Game;
class Dependency;
DECLARE_POINTER_TYPE(Card);
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
	
	/// Find a value in the data by name and type
	template <typename T> T& value(const String& name) {
		for(IndexMap<FieldP, ValueP>::iterator it = data.begin() ; it != data.end() ; ++it) {
			if ((*it)->fieldP->name == name) {
				T* ret = dynamic_cast<T*>(it->get());
				if (!ret) throw InternalError(_("Card field with name '")+name+_("' doesn't have the right type"));
				return *ret;
			}
		}
		throw InternalError(_("Expected a card field with name '")+name+_("'"));
	}
	template <typename T> const T& value(const String& name) const {
		for(IndexMap<FieldP, ValueP>::const_iterator it = data.begin() ; it != data.end() ; ++it) {
			if ((*it)->fieldP->name == name) {
				const T* ret = dynamic_cast<const T*>(it->get());
				if (!ret) throw InternalError(_("Card field with name '")+name+_("' doesn't have the right type"));
				return *ret;
			}
		}
		throw InternalError(_("Expected a card field with name '")+name+_("'"));
	}
	
	DECLARE_REFLECTION();
};

void mark_dependency_member(const Card& value, const String& name, const Dependency& dep);

// ----------------------------------------------------------------------------- : EOF
#endif
