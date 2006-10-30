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
#include <data/field/text.hpp> // for 0.2.7 fix
#include <script/value.hpp>
#include <script/script_manager.hpp>

DECLARE_TYPEOF_COLLECTION(CardP);
typedef IndexMap<FieldP,ValueP> IndexMap_FieldP_ValueP;
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_ValueP);

// ----------------------------------------------------------------------------- : Set

Set::Set()
	: script_manager(new ScriptManager(*this))
{}

Set::Set(const GameP& game)
	: game(game)
	, script_manager(new ScriptManager(*this))
{}

Set::Set(const StyleSheetP& stylesheet)
	: stylesheet(stylesheet)
	, game(stylesheet->game)
	, script_manager(new ScriptManager(*this))
{}

Set::~Set() {}


Context& Set::getContext() {
	return script_manager->getContext(stylesheet);
}
Context& Set::getContext(const Card& card) {
	return script_manager->getContext(card.stylesheet ? card.stylesheet : stylesheet);
}

String Set::typeName() const { return _("set"); }

// fix values for versions < 0.2.7
void fix_value_207(const ValueP& value) {
	if (TextValue* v = dynamic_cast<TextValue*>(value.get())) {
		// text value -> fix it
//		v->value.assign(	// don't change defaultness
//			fix_old_tags(v->value); // remove tags
//		);
	}
}

void Set::validate(Version file_app_version) {
	// are the
	if (!game) {
		throw Error(_("No game specified for the set"));
	}
	if (!stylesheet) {
		// TODO : Allow user to select a different style
		throw Error(_("No stylesheet specified for the set"));
	}
	if (stylesheet->game != game) {
		throw Error(_("stylesheet and set don't refer to the same game, this is an error in the stylesheet file"));
	}
	
	// This is our chance to fix version incompatabilities
	if (file_app_version < 207) {
		// Since 0.2.7 we use </tag> style close tags, in older versions it was </>
		// Walk over all fields and fix...
		FOR_EACH(c, cards) {
			FOR_EACH(v, c->data) fix_value_207(v);
		}
		FOR_EACH(v, data) fix_value_207(v);
/*		FOR_EACH(s, styleData) {
			FOR_EACH(v, s.second->data) fix_value_207(v);
		}
*/	}
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
