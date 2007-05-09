//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_PACK
#define HEADER_DATA_PACK

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(CardType);
DECLARE_POINTER_TYPE(Card);
class Set;

// ----------------------------------------------------------------------------- : PackType

/// A card pack description for playtesting
class PackType {
  public:
	PackType();
	
	String            name;			///< Name of this pack
	vector<CardTypeP> card_types;	///< Cards in this pack
	Scriptable<bool>  enabled;		///< Is this pack enabled?
	
	/// Generate a random pack of cards
	void generate(Set& set, vector<CardP>& out);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : CardType

/// A card type description for playtesting
class CardType {
  public:
	String          name;	///< Name of this type of cards
	Scriptable<int> amount;	///< Number of cards of this type
	OptionalScript  filter;	///< Filter to select this type of cards
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
