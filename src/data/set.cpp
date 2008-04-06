//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/card.hpp>
#include <data/keyword.hpp>
#include <data/field.hpp>
#include <data/field/text.hpp>    // for 0.2.7 fix
#include <data/field/information.hpp>
#include <util/tagged_string.hpp> // for 0.2.7 fix
#include <util/order_cache.hpp>
#include <script/script_manager.hpp>
#include <wx/sstream.h>

DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_NO_REV(IndexMap<FieldP COMMA ValueP>);

// ----------------------------------------------------------------------------- : Set

Set::Set()
	: script_manager(new SetScriptManager(*this))
{}

Set::Set(const GameP& game)
	: game(game)
	, script_manager(new SetScriptManager(*this))
{
	data.init(game->set_fields);
}

Set::Set(const StyleSheetP& stylesheet)
	: game(stylesheet->game)
	, stylesheet(stylesheet)
	, script_manager(new SetScriptManager(*this))
{
	data.init(game->set_fields);
}

Set::~Set() {}


Context& Set::getContext() {
	assert(wxThread::IsMain());
	return script_manager->getContext(stylesheet);
}
Context& Set::getContext(const CardP& card) {
	assert(wxThread::IsMain());
	return script_manager->getContext(card);
}
void Set::updateStyles(const CardP& card, bool only_content_dependent) {
	script_manager->updateStyles(card, only_content_dependent);
}
void Set::updateDelayed() {
	script_manager->updateDelayed();
}

Context& Set::getContextForThumbnails() {
	assert(!wxThread::IsMain());
	if (!thumbnail_script_context) {
		thumbnail_script_context.reset(new SetScriptContext(*this));
	}
	return thumbnail_script_context->getContext(stylesheet);
}
Context& Set::getContextForThumbnails(const CardP& card) {
	assert(!wxThread::IsMain());
	if (!thumbnail_script_context) {
		thumbnail_script_context.reset(new SetScriptContext(*this));
	}
	return thumbnail_script_context->getContext(card);
}
Context& Set::getContextForThumbnails(const StyleSheetP& stylesheet) {
	assert(!wxThread::IsMain());
	if (!thumbnail_script_context) {
		thumbnail_script_context.reset(new SetScriptContext(*this));
	}
	return thumbnail_script_context->getContext(stylesheet);
}

const StyleSheet& Set::stylesheetFor(const CardP& card) {
	if (card && card->stylesheet) return *card->stylesheet;
	else                          return *stylesheet;
}
StyleSheetP Set::stylesheetForP(const CardP& card) {
	if (card && card->stylesheet) return card->stylesheet;
	else                          return stylesheet;
}

IndexMap<FieldP, ValueP>& Set::stylingDataFor(const StyleSheet& stylesheet) {
	return styling_data.get(stylesheet.name(), stylesheet.styling_fields);
}
IndexMap<FieldP, ValueP>& Set::stylingDataFor(const CardP& card) {
	if (card && card->has_styling) return card->styling_data;
	else                           return stylingDataFor(stylesheetFor(card));
}

String Set::identification() const {
	// an identifying field
	FOR_EACH_CONST(v, data) {
		if (v->fieldP->identifying) {
			return v->toString();
		}
	}
	// otherwise the first non-information field
	FOR_EACH_CONST(v, data) {
		if (!dynamic_pointer_cast<InfoValue>(v)) {
			return v->toString();
		}
	}
	return wxEmptyString;
}


String Set::typeName() const { return _("set"); }

// fix values for versions < 0.2.7
void fix_value_207(const ValueP& value) {
	if (TextValue* v = dynamic_cast<TextValue*>(value.get())) {
		// text value -> fix it
		v->value.assignDontChangeDefault(	// don't change defaultness
			fix_old_tags(v->value()) // remove tags
		);
	}
}

void Set::validate(Version file_app_version) {
	Packaged::validate(file_app_version);
	// are the
	if (!game) {
		throw Error(_ERROR_1_("no game specified",_TYPE_("set")));
	}
	if (!stylesheet) {
		// TODO : Allow user to select a different style
		throw Error(_ERROR_("no stylesheet specified for the set"));
	}
	if (stylesheet->game != game) {
		throw Error(_ERROR_("stylesheet and set refer to different game"));
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
	// we want at least one card
	if (cards.empty()) cards.push_back(new_intrusive1<Card>(*game));
	// update scripts
	script_manager->updateAll();
}

IMPLEMENT_REFLECTION(Set) {
	REFLECT_ALIAS(300, "style",          "stylesheet"); // < 0.3.0 used style instead of stylesheet
	REFLECT_ALIAS(300, "extra set info", "styling");
	REFLECT(game);
	if (game) {
		REFLECT_IF_READING {
			data.init(game->set_fields);
		}
		WITH_DYNAMIC_ARG(game_for_reading, game.get());
		REFLECT(stylesheet);
		WITH_DYNAMIC_ARG(stylesheet_for_reading, stylesheet.get());
		REFLECT_N("set_info", data);
		if (stylesheet) {
			REFLECT_N("styling", styling_data);
		}
		REFLECT(cards);
		REFLECT(keywords);
	}
	reflect_set_info_get_member(tag,data);
	REFLECT(apprentice_code);
}

// ----------------------------------------------------------------------------- : Script utilities

ScriptValueP make_iterator(const Set& set) {
	return new_intrusive1<ScriptCollectionIterator<vector<CardP> > >(&set.cards);
}

void mark_dependency_member(const Set& set, const String& name, const Dependency& dep) {
	// is it the card list?
	if (name == _("cards")) {
		set.game->dependent_scripts_cards.add(dep);
		return;
	}
	// is it the keywords?
	if (name == _("keywords")) {
		set.game->dependent_scripts_keywords.add(dep);
		return;
	}
	// is it in the set data?
	mark_dependency_member(set.data, name, dep);
}

// in scripts, set.something is read from the set_info
template <typename Tag>
void reflect_set_info_get_member(Tag&       tag, const IndexMap<FieldP, ValueP>& data) {}
void reflect_set_info_get_member(GetMember& tag, const IndexMap<FieldP, ValueP>& data) {
	REFLECT_NAMELESS(data);
}

int Set::positionOfCard(const CardP& card, const ScriptValueP& order_by, const ScriptValueP& filter) {
	// TODO : Lock the map?
	assert(order_by);
	OrderCacheP& order = order_cache[make_pair(order_by,filter)];
	if (!order) {
		// 1. make a list of the order value for each card
		vector<String> values; values.reserve(cards.size());
		vector<int>    keep;   if(filter) keep.reserve(cards.size());
		FOR_EACH_CONST(c, cards) {
			Context& ctx = getContext(c);
			values.push_back(*order_by->eval(ctx));
			if (filter) {
				keep.push_back(*filter->eval(ctx));
			}
		}
		// 3. initialize order cache
		order = new_intrusive3<OrderCache<CardP> >(cards, values, filter ? &keep : nullptr);
	}
	return order->find(card);
}
int Set::numberOfCards(const ScriptValueP& filter) {
	if (!filter) return (int)cards.size();
	map<ScriptValueP,int>::const_iterator it = filter_cache.find(filter);
	if (it !=filter_cache.end()) {
		return it->second;
	} else {
		int n = 0;
		FOR_EACH_CONST(c, cards) {
			if (*filter->eval(getContext(c))) ++n;
		}
		filter_cache.insert(make_pair(filter,n));
		return n;
	}
}
void Set::clearOrderCache() {
	order_cache.clear();
	filter_cache.clear();
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
