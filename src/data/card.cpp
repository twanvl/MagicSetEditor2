//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/card.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/field.hpp>
#include <util/error.hpp>
#include <util/reflect.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_NO_REV(IndexMap<FieldP COMMA ValueP>);

// ----------------------------------------------------------------------------- : Card

Card::Card() {
	if (!game_for_reading()) {
		throw InternalError(_("game_for_reading not set"));
	}
	data.init(game_for_reading()->card_fields);
}

Card::Card(const Game& game) {
	data.init(game.card_fields);
}

String Card::identification() const {
	// an identifying field
	FOR_EACH_CONST(v, data) {
		if (v->fieldP->identifying) {
			return v->toString();
		}
	}
	// otherwise the first field
	if (!data.empty()) {
		return data.at(0)->toString();
	} else {
		return wxEmptyString;
	}
}

void mark_dependency_member(const Card& card, const String& name, const Dependency& dep) {
	mark_dependency_member(card.data, name, dep);
}

IMPLEMENT_REFLECTION(Card) {
	REFLECT(stylesheet);
	REFLECT(notes);
	REFLECT_NAMELESS(data);
}

