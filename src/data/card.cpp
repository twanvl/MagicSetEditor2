//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/card.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/field.hpp>
#include <util/error.hpp>
#include <util/reflect.hpp>
#include <util/delayed_index_maps.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_NO_REV(IndexMap<FieldP COMMA ValueP>);

// ----------------------------------------------------------------------------- : Card

Card::Card()
	  // for files made before we saved these times, set the time to 'yesterday'
	: time_created (wxDateTime::Now().Subtract(wxDateSpan::Day()).ResetTime())
	, time_modified(wxDateTime::Now().Subtract(wxDateSpan::Day()).ResetTime())
	, has_styling(false)
{
	if (!game_for_reading()) {
		throw InternalError(_("game_for_reading not set"));
	}
	data.init(game_for_reading()->card_fields);
}

Card::Card(const Game& game)
	: time_created (wxDateTime::Now())
	, time_modified(wxDateTime::Now())
	, has_styling(false)
{
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

/// Does the given object match the quick search query?
template <typename T>
bool match_quicksearch_query(String const& query, T const& object) {
	bool need_match = true;
	// iterate over the components of the query
	for (size_t i = 0 ; i < query.size() ; ) {
		if (query.GetChar(i) == _(' ')) {
			// skip spaces
			i++;
		} else if (query.GetChar(i) == _('-')) {
			// negate the next query, i.e. match only if it is not on the card
			need_match = !need_match;
			i++;
		} else {
			size_t end, next;
			if (query.GetChar(i) == _('"')) {
				// quoted string, match exactly
				i++;
				end =query.find_first_of(_('"'),i);
				next = min(end,query.size()) + 1;
			} else {
				// single word
				next = end = query.find_first_of(_(' '),i);
			}
			bool match = object.contains(query.substr(i,end-i));
			if (match != need_match) {
				return false;
			}
			need_match = true; // next word is no longer negated
			i = next;
		}
	}
	return true;
}

bool Card::contains(String const& query) const {
	FOR_EACH_CONST(v, data) {
		if (find_i(v->toString(),query) != String::npos) return true;
	}
	if (find_i(notes,query) != String::npos) return true;
	return false;
}
bool Card::contains_words(String const& query) const {
	return match_quicksearch_query(query,*this);
}

IndexMap<FieldP, ValueP>& Card::extraDataFor(const StyleSheet& stylesheet) {
	return extra_data.get(stylesheet.name(), stylesheet.extra_card_fields);
}

void mark_dependency_member(const Card& card, const String& name, const Dependency& dep) {
	mark_dependency_member(card.data, name, dep);
}

IMPLEMENT_REFLECTION(Card) {
	REFLECT(stylesheet);
	REFLECT(has_styling);
	if (has_styling) {
		if (stylesheet) {
			REFLECT_IF_READING styling_data.init(stylesheet->styling_fields);
			REFLECT(styling_data);
		} else if (stylesheet_for_reading()) {
			REFLECT_IF_READING styling_data.init(stylesheet_for_reading()->styling_fields);
			REFLECT(styling_data);
		} else if (tag.reading()) {
			has_styling = false; // We don't know the style, this can be because of copy/pasting
		}
	}
	REFLECT(notes);
	REFLECT(time_created);
	REFLECT(time_modified);
	REFLECT(extra_data); // don't allow scripts to depend on style specific data
	REFLECT_NAMELESS(data);
}
