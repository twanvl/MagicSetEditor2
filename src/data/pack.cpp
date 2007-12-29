//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/pack.hpp>

// ----------------------------------------------------------------------------- : PackType

PackType::PackType()
	: enabled(true)
{}

IMPLEMENT_REFLECTION(PackType) {
	REFLECT(name);
	REFLECT(enabled);
	REFLECT(card_types);
}

// ----------------------------------------------------------------------------- : CardType

IMPLEMENT_REFLECTION(CardType) {
	REFLECT(name);
	REFLECT(amount);
	REFLECT(filter);
}
