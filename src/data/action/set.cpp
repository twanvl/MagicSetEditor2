//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/action/set.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(IndexMap<FieldP COMMA ValueP>);

// ----------------------------------------------------------------------------- : Add card

AddCardAction::AddCardAction(Set& set)
	: CardListAction(set), card(new Card(*set.game))
{}

AddCardAction::AddCardAction(Set& set, const CardP& card)
	: CardListAction(set), card(card)
{}

String AddCardAction::getName(bool to_undo) const {
	return _("Add card");
}

void AddCardAction::perform(bool to_undo) {
	if (!to_undo) {
		set.cards.push_back(card);
	} else {
		assert(!set.cards.empty());
		set.cards.pop_back();
	}
}


// ----------------------------------------------------------------------------- : Remove card

RemoveCardAction::RemoveCardAction(Set& set, const CardP& card)
	: CardListAction(set), card(card)
	// find the card_id of the card we want to remove
	, card_id(find(set.cards.begin(), set.cards.end(), card) - set.cards.begin())
{
	if (card_id >= set.cards.size()) {
		throw InternalError(_("Card to remove not found in set"));
	}
}

String RemoveCardAction::getName(bool to_undo) const {
	return _("Remove card");
}

void RemoveCardAction::perform(bool to_undo) {
	if (!to_undo) {
		assert(card_id < set.cards.size());
		set.cards.erase(set.cards.begin() + card_id);
	} else {
		assert(card_id <= set.cards.size());
		set.cards.insert(set.cards.begin() + card_id, card);
	}
}


// ----------------------------------------------------------------------------- : Reorder cards

ReorderCardsAction::ReorderCardsAction(Set& set, size_t card_id1, size_t card_id2)
	: CardListAction(set), card_id1(card_id1), card_id2(card_id2)
{}

String ReorderCardsAction::getName(bool to_undo) const {
	return _("Reorder cards");
}

void ReorderCardsAction::perform(bool to_undo) {
	assert(card_id1 < set.cards.size());
	assert(card_id2 < set.cards.size());
	swap(set.cards[card_id1], set.cards[card_id2]);
}

// ----------------------------------------------------------------------------- : Change stylesheet

String DisplayChangeAction::getName(bool to_undo) const {
	assert(false);
	return _("");
}
void DisplayChangeAction::perform(bool to_undo) {
	assert(false);
}


ChangeCardStyleAction::ChangeCardStyleAction(const CardP& card, const StyleSheetP& stylesheet)
	: card(card), stylesheet(stylesheet), has_styling(false) // styling_data(empty)
{}
String ChangeCardStyleAction::getName(bool to_undo) const {
	return _("Change style");
}
void ChangeCardStyleAction::perform(bool to_undo) {
	swap(card->stylesheet,   stylesheet);
	swap(card->has_styling,  has_styling);
	swap(card->styling_data, styling_data);
}


ChangeSetStyleAction::ChangeSetStyleAction(Set& set, const CardP& card)
	: set(set), card(card)
{}
String ChangeSetStyleAction::getName(bool to_undo) const {
	return _("Change style (all cards)");
}
void ChangeSetStyleAction::perform(bool to_undo) {
	if (!to_undo) {
		stylesheet       = set.stylesheet;
		set.stylesheet   = card->stylesheet;
		card->stylesheet = StyleSheetP();
	} else {
		card->stylesheet = set.stylesheet;
		set.stylesheet   = stylesheet;
	}
}

ChangeCardHasStylingAction::ChangeCardHasStylingAction(Set& set, const CardP& card)
	: set(set), card(card)
{
	if (!card->has_styling) {
		// copy of the set's styling data
		styling_data.cloneFrom( set.stylingDataFor(set.stylesheetFor(card)) );
	} else {
		// the new styling data is empty
	}
}
String ChangeCardHasStylingAction::getName(bool to_undo) const {
	return _("Use custom style");
}
void ChangeCardHasStylingAction::perform(bool to_undo) {
	card->has_styling = !card->has_styling;
	swap(card->styling_data, styling_data);
}
