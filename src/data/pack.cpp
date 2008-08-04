//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/pack.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>

DECLARE_TYPEOF_COLLECTION(PackItemRefP);
DECLARE_TYPEOF_COLLECTION(PackItemP);
DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : PackType

PackType::PackType()
	: enabled(true)
{}

IMPLEMENT_REFLECTION(PackType) {
	REFLECT(name);
	REFLECT(enabled);
	REFLECT(items);
}

bool PackType::update(Context& ctx) {
	bool change = enabled.update(ctx);
	FOR_EACH(item, items) {
		change |= item->update(ctx);
	}
	return change;
}

void PackType::generate(PackItemCache& packs, boost::mt19937& gen, vector<CardP>& out) const {
	FOR_EACH_CONST(item, items) {
		item->generate(packs,gen,out);
	}
}

// ----------------------------------------------------------------------------- : PackRefType

IMPLEMENT_REFLECTION_ENUM(PackRefType) {
	VALUE_N("replace",    PACK_REF_REPLACE);
	VALUE_N("no replace", PACK_REF_NO_REPLACE);
	VALUE_N("cyclic",     PACK_REF_CYCLIC);
}

// ----------------------------------------------------------------------------- : PackItemRef

PackItemRef::PackItemRef()
	: amount(1)
	, type(PACK_REF_REPLACE)
{}

IMPLEMENT_REFLECTION(PackItemRef) {
	REFLECT(name);
	REFLECT(amount);
	REFLECT(type);
}

bool PackItemRef::update(Context& ctx) {
	return amount.update(ctx);
}

/// Random generator with random numbers in a range
template <typename Gen>
struct RandomRange {
	RandomRange(Gen& gen) : gen(gen) {}
	unsigned operator () (unsigned max) { return gen() % max; }
	Gen& gen;
};

void PackItemRef::generate(PackItemCache& packs, boost::mt19937& gen, vector<CardP>& out) const {
	vector<CardP>& cards = packs.cardsFor(name);
	// generate 'amount' cards and add them to out
	if (cards.empty()) return;
	if (type == PACK_REF_REPLACE) {
		// amount random numbers
		for (int i = 0 ; i < amount ; ++i) {
			size_t index = gen() % cards.size();
			out.push_back(cards[index]);
		}
	} else if (type == PACK_REF_NO_REPLACE) {
		// random shuffle
		// to prevent us from being too predictable for small sets, periodically reshuffle
		RandomRange<boost::mt19937> gen_range(gen);
		size_t max_per_batch = (cards.size() + 1) / 2;
		int rem = amount;
		while (rem > 0) {
			random_shuffle(cards.begin(), cards.end(), gen_range);
			out.insert(out.end(),  cards.begin(), cards.begin() + min((size_t)rem, max_per_batch));
			rem -= (int)max_per_batch;
		}
	} else if (type == PACK_REF_CYCLIC) {
		// multiple copies
		size_t copies = amount / cards.size();
		FOR_EACH_CONST(card, cards) {
			out.insert(out.end(),copies,card);
		}
		// TODO: what if amount is not a multiple of the number of cards?
	}
}

// ----------------------------------------------------------------------------- : PackItem

IMPLEMENT_REFLECTION(PackItem) {
	REFLECT(name);
	REFLECT(filter);
}

void PackItem::generate(Set& set, vector<CardP>& out) const {
	FOR_EACH(card, set.cards) {
		Context& ctx = set.getContext(card);
		bool keep = *filter.invoke(ctx);
		if (keep) {
			out.push_back(card);
		}
	}
}

// ----------------------------------------------------------------------------- : PackItemCache

PackItemCache::PackItemCache(Set& set)
	: set(set)
{}

vector<CardP>& PackItemCache::cardsFor(const String& name) {
	// lookup name
	map<String,Cards>::iterator it = item_cards.find(name);
	if (it != item_cards.end()) {
		return *it->second;
	} else {
		// not used before, generate list and cache
		FOR_EACH(item, set.game->pack_items) {
			if (item->name == name) {
				Cards cards(new vector<CardP>);
				item->generate(set,*cards);
				item_cards.insert(make_pair(name,cards));
				return *cards;
			}
		}
		// still not found
		throw Error(_ERROR_1_("pack item not found",name));
	}
}
