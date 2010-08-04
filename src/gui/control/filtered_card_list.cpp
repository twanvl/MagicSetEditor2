//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/filtered_card_list.hpp>

DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : FilteredCardList

FilteredCardList::FilteredCardList(Window* parent, int id, long additional_style)
	: CardListBase(parent, id, additional_style)
{}

void FilteredCardList::setFilter(const CardListFilterP& filter) {
	this->filter = filter;
	rebuild();
}

void FilteredCardList::onChangeSet() {
	// clear filter before changing set, the filter might not make sense for a different set
	filter = CardListFilterP();
	CardListBase::onChangeSet();
}

void FilteredCardList::getItems(vector<VoidP>& out) const {
	if (filter) {
		filter->getItems(set->cards,out);
	}
}

// ----------------------------------------------------------------------------- : CardListFilter

void CardListFilter::getItems(const vector<CardP>& cards, vector<VoidP>& out) const {
	FOR_EACH_CONST(c, cards) {
		if (keep(c)) {
			out.push_back(c);
		}
	}
}

bool QueryCardListFilter::keep(const CardP& card) const {
	return card->contains(query);
}

