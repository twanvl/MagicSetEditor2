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
#include <wx/sstream.h>

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
	REFLECT_NO_SCRIPT(extra_data);
	REFLECT_NAMELESS(data);
}

// ----------------------------------------------------------------------------- : Styling

// TODO : this is practically the same as Set::Styling, maybe somehow abstract it

// Extra card data, for a specific stylesheet
/* The data is not read immediatly, because we do not know the stylesheet */
class Card::Styling : public IntrusivePtrBase<Card::Styling> {
  public:
	/// The values on the extra card fields of the card.
	/** The indices should correspond to the extra_card_fields in the StyleSheet */
	IndexMap<FieldP, ValueP> extra_data;
	/// Unparsed extra_data
	String unread_data;
	DECLARE_REFLECTION();
};

IndexMap<FieldP, ValueP>& Card::extraDataFor(const StyleSheet& stylesheet) {
	StylingP& styling = extra_data[stylesheet.name()];
	if (!styling) {
		styling = new_intrusive<Styling>();
		styling->extra_data.init(stylesheet.extra_card_fields);
	} else if (!styling->unread_data.empty() || (styling->extra_data.empty()) && !stylesheet.extra_card_fields.empty()) {
		// we delayed the reading of the data, read it now
		styling->extra_data.init(stylesheet.extra_card_fields);
		Reader reader(new_shared1<wxStringInputStream>(styling->unread_data), _("extra card values for ") + stylesheet.stylesheetName());
		reader.handle_greedy(styling->extra_data);
		styling->unread_data.clear();
	}
	return styling->extra_data;
}

// custom reflection : read into unread_data
template <> void Reader::handle(Card::Styling& s) {
	handle(s.unread_data);
}
template <> void Writer::handle(const Card::Styling& s) {
	handle(s.extra_data);
}
// for the love of god, don't depend on styling
template <> void GetMember::handle(const Card::Styling& s) {}
template <> void GetDefaultMember::handle(const Card::Styling& s) {}
