//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/filtered_card_list.hpp>

DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : FilteredCardList

FilteredCardList::FilteredCardList(Window* parent, int id, int style)
	: CardListBase(parent, id, style)
{}

const vector<CardP>& FilteredCardList::getCards() const {
	return cards;
}

void FilteredCardList::setFilter(const CardListFilterP& filter_) {
	filter = filter_;
	rebuild();
}

void FilteredCardList::onRebuild() {
	cards.clear();
	if (filter) {
		FOR_EACH(c, set->cards) {
			if (filter->keep(c)) {
				cards.push_back(c);
			}
		}
	}
}
