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
#include <data/keyword.hpp>
#include <data/field.hpp>
#include <data/field/text.hpp>    // for 0.2.7 fix
#include <util/tagged_string.hpp> // for 0.2.7 fix
#include <util/order_cache.hpp>
#include <script/value.hpp>
#include <script/script_manager.hpp>
#include <wx/sstream.h>

DECLARE_TYPEOF_COLLECTION(CardP);
typedef IndexMap<FieldP,ValueP> IndexMap_FieldP_ValueP;
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_ValueP);

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
void Set::updateFor(const CardP& card) {
	script_manager->updateStyles(card);
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

StyleSheetP Set::stylesheetFor(const CardP& card) {
	if (card && card->stylesheet) return card->stylesheet;
	else                          return stylesheet;
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
	// are the
	if (!game) {
		throw Error(_ERROR_("no game specified for the set"));
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
	if (cards.empty()) cards.push_back(new_shared1<Card>(*game));
	// update scripts
	script_manager->updateAll();
}

IMPLEMENT_REFLECTION(Set) {
	tag.addAlias(300, _("style"),          _("stylesheet")); // < 0.3.0 used style instead of stylesheet
	tag.addAlias(300, _("extra set info"), _("styling"));
	REFLECT(game);
	if (game) {
		if (tag.reading()) {
			data.init(game->set_fields);
		}
		WITH_DYNAMIC_ARG(game_for_reading, game.get());
		REFLECT(stylesheet);
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

void mark_dependency_member(Set* value, const String& name, const Dependency& dep) {
	// is it the card list?
	if (name == _("cards")) {
		value->game->dependent_scripts_cards.add(dep);
		return;
	}
	// is it the keywords?
	if (name == _("keywords")) {
		value->game->dependent_scripts_keywords.add(dep);
		return;
	}
	// is it in the set data?
	IndexMap<FieldP,ValueP>::const_iterator it = value->data.find(name);
	if (it != value->data.end()) {
		(*it)->fieldP->dependent_scripts.add(dep);
	}
}
void mark_dependency_member(const SetP& value, const String& name, const Dependency& dep) {
	mark_dependency_member(value.get(), name, dep);
}

// in scripts, set.something is read from the set_info
template <typename Tag>
void reflect_set_info_get_member(Tag&       tag, const IndexMap<FieldP, ValueP>& data) {}
void reflect_set_info_get_member(GetMember& tag, const IndexMap<FieldP, ValueP>& data) {
	REFLECT_NAMELESS(data);
}

int Set::positionOfCard(const CardP& card, const ScriptValueP& order_by) {
	// TODO : Lock the map?
	assert(order_by);
	OrderCacheP& order = order_cache[order_by];
	if (!order) {
		// 1. make a list of the order value for each card
		vector<String> values; values.reserve(cards.size());
		FOR_EACH_CONST(c, cards) {
			values.push_back(*order_by->eval(getContext(c)));
		}
		// 2. initialize order cache
		order.reset(new OrderCache<CardP>(cards, values));
	}
	return order->find(card);
}
void Set::clearOrderCache() {
	order_cache.clear();
}

// ----------------------------------------------------------------------------- : Styling

// Extra set data, for a specific stylesheet
/* The data is not read immediatly, because we do not know the stylesheet */
class Set::Styling {
  public:
	IndexMap<FieldP, ValueP> data;
	String unread_data;
	DECLARE_REFLECTION();
};

IndexMap<FieldP, ValueP>& Set::stylingDataFor(const StyleSheet& stylesheet) {
	StylingP& styling = styling_data[stylesheet.name()];
	if (!styling) {
		styling = new_shared<Styling>();
		styling->data.init(stylesheet.styling_fields);
	} else if (!styling->unread_data.empty()) {
		// we delayed the reading of the data, read it now
		styling->data.init(stylesheet.styling_fields);
		Reader reader(new_shared1<wxStringInputStream>(styling->unread_data), _("styling data of ") + stylesheet.stylesheetName());
		reader.handle_greedy(styling->data);
		styling->unread_data.clear();
	}
	return styling->data;
}

// custom reflection : read into unread_data
template <> void Reader::handle(Set::Styling& s) {
	handle(s.unread_data);
}
template <> void Writer::handle(const Set::Styling& s) {
	handle(s.data);
}
template <> void GetMember::handle(const Set::Styling& s) {
	handle(s.data);
}
template <> void GetDefaultMember::handle(const Set::Styling& s) {
	handle(s.data);
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
