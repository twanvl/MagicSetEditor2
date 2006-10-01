//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/set.hpp>
#include <data/card.hpp>

// ----------------------------------------------------------------------------- : Set

IMPLEMENT_REFLECTION(Set) {
	WITH_DYNAMIC_ARG(game_for_new_cards, game.get()) {
		REFLECT_N("card", cards);
	}
}

// ----------------------------------------------------------------------------- : SetView

SetView::SetView() {}

SetView::~SetView() {
	if (set) set->actions.removeListener(this);
}

void SetView::setSet(const SetP& newSet) {
	// no longer listening to old set
	if (set) set->actions.removeListener(this);
	set = newSet;
	// start listening to new set
	if (set) set->actions.addListener(this);
	onChangeSet();
}
