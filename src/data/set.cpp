//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/field.hpp>
#include <script/value.hpp>

// ----------------------------------------------------------------------------- : Set

Set::Set(const GameP& game)
	: game(game)
{}

Set::Set() {}

String Set::typeName() const { return _("set"); }

IMPLEMENT_REFLECTION(Set) {
	REFLECT(game);
	if (data.empty() && game) {
		data.init(game->set_fields);
	}
	REFLECT_N("set_info", data);
	WITH_DYNAMIC_ARG(game_for_new_cards, game.get()) {
		REFLECT(cards);
	}
	REFLECT(apprentice_code);
}


// ----------------------------------------------------------------------------- : SetView

SetView::SetView() {}

SetView::~SetView() {
	if (set) set->actions.removeListener(this);
}

void SetView::setSet(const SetP& newSet) {
	// no longer listening to old set
	if (set) {
		onBeforeChangeSet();
		set->actions.removeListener(this);
	}
	set = newSet;
	// start listening to new set
	if (set) set->actions.addListener(this);
	onChangeSet();
}
