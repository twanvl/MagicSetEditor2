//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/card.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Card

IMPLEMENT_DYNAMIC_ARG(Game*, game_for_new_cards, nullptr);

Card::Card() {
	if (!game_for_new_cards()) {
		throw InternalError(_("game_for_new_cards not set"));
	}
	data.init(game_for_new_cards()->cardFields);
}

Card::Card(const Game& game) {
	data.init(game.cardFields);
}

String Card::identification() const {
	return _("TODO");
}

IMPLEMENT_REFLECTION(Card) {
	REFLECT(notes);
}

