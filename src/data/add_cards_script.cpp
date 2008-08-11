//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/add_cards_script.hpp>
#include <data/action/set.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>

// ----------------------------------------------------------------------------- : AddCardsScript

IMPLEMENT_REFLECTION_NO_SCRIPT(AddCardsScript) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(enabled);
	REFLECT(script);
}


void AddCardsScript::perform(Set& set, vector<CardP>& out) {
	// Perform script
	Context& ctx = set.getContext();
	ScriptValueP result = script.invoke(ctx);
	// Add cards to out
	ScriptValueP it = result->makeIterator(result);
	while (ScriptValueP item = it->next()) {
		CardP card = from_script<CardP>(item);
		// is this a new card?
		if (contains(set.cards,card) || contains(out,card)) {
			// make copy
			card = new_intrusive1<Card>(*card);
		}
		out.push_back(card);
	}
}

void AddCardsScript::perform(Set& set) {
	// Perform script
	vector<CardP> cards;
	perform(set,cards);
	// Add to set
	if (!cards.empty()) {
		// TODO: change the name of the action somehow
		set.actions.addAction(new AddCardAction(ADD, set, cards));
	}
}
