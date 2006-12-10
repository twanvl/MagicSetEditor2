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
#include <script/value.hpp>
#include <script/script_manager.hpp>
#include <wx/sstream.h>

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
{
	data.init(game->set_fields);
}

Set::Set(const StyleSheetP& stylesheet)
	: stylesheet(stylesheet)
	, game(stylesheet->game)
	, script_manager(new ScriptManager(*this))
{
	data.init(game->set_fields);
}

Set::~Set() {}


Context& Set::getContext() {
	return script_manager->getContext(stylesheet);
}
Context& Set::getContext(const CardP& card) {
	return script_manager->getContext(card);
}
void Set::updateFor(const CardP& card) {
	script_manager->updateStyles(card);
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

void mark_dependency_member(Set* value, const String& name, const Dependency& dep) {
	// TODO
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
		reader.handle(styling->data);
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
