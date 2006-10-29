//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/card.hpp>
#include <data/field.hpp>
#include <script/value.hpp>
#include <script/script_manager.hpp>

// ----------------------------------------------------------------------------- : Set

Set::Set() {}

Set::Set(const GameP& game)
	: game(game)
{}

Set::Set(const StyleSheetP& stylesheet)
	: stylesheet(stylesheet)
	, game(stylesheet->game)
{}

Set::~Set() {}


Context& Set::getContext() {
	throw "TODO";
}

String Set::typeName() const { return _("set"); }

void Set::validate() {
}

IMPLEMENT_REFLECTION(Set) {
	tag.addAlias(300, _("style"), _("stylesheet")); // < 0.3.0 used style instead of stylesheet
	REFLECT(game);
	if (game) {
		if (tag.reading()) {
			data.init(game->set_fields);
		}
		WITH_DYNAMIC_ARG(game_for_reading, game.get());
		REFLECT(stylesheet);
		REFLECT_N("set_info", data);
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
