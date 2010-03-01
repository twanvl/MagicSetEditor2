//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/pack.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <queue>

#if !USE_NEW_PACK_SYSTEM
// =================================================================================================== OLD

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
	, type(PACK_REF_NO_REPLACE)
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




#else
// =================================================================================================== NEW

DECLARE_TYPEOF_COLLECTION(PackTypeP);
DECLARE_TYPEOF_COLLECTION(PackItemP);
DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_CONST(map<String COMMA PackInstanceP>);

// ----------------------------------------------------------------------------- : PackType


IMPLEMENT_REFLECTION_ENUM(PackSelectType) {
	VALUE_N("auto",               SELECT_AUTO);
	VALUE_N("all",                SELECT_ALL);
	VALUE_N("no replace",         SELECT_NO_REPLACE);
	VALUE_N("replace",            SELECT_REPLACE);
	VALUE_N("proportional",       SELECT_PROPORTIONAL);
	VALUE_N("nonempty",           SELECT_NONEMPTY);
	VALUE_N("equal",              SELECT_EQUAL);
	VALUE_N("equal proportional", SELECT_EQUAL_PROPORTIONAL);
	VALUE_N("equal nonempty",     SELECT_NONEMPTY);
	VALUE_N("first",              SELECT_FIRST);
}

IMPLEMENT_REFLECTION(PackType) {
	REFLECT(name);
	REFLECT(enabled);
	REFLECT(selectable);
	REFLECT(summary);
	REFLECT(select);
	REFLECT(filter);
	REFLECT(items);
	REFLECT_IF_READING {
		if (select == SELECT_AUTO) {
			if (filter)              select = SELECT_NO_REPLACE;
			else if (!items.empty()) select = SELECT_ALL;
		}
		if (indeterminate(summary)) {
			if (filter)              summary = true;
			else if (!items.empty()) summary = false;
		}
		if (indeterminate(selectable)) {
			if (filter)              selectable = false;
			else if (!items.empty()) selectable = true;
		}
	}
}

IMPLEMENT_REFLECTION(PackItem) {
	if (!tag.isComplex()) {
		REFLECT_NAMELESS(name);
	} else {
		REFLECT(name);
		REFLECT(amount);
		REFLECT(weight);
	}
}


PackType::PackType()
	: enabled(true)
	, selectable(indeterminate)
	, summary(indeterminate)
	, select(SELECT_AUTO)
{}

PackItem::PackItem()
	: amount(1)
	, weight(1)
{}

PackItem::PackItem(const String& name, int amount)
	: name(name)
	, amount(amount)
	, weight(1)
{}


bool PackType::update(Context& ctx) {
	bool change = enabled.update(ctx);
	FOR_EACH(item, items) {
		change |= item->update(ctx);
	}
	return change;
}

bool PackItem::update(Context& ctx) {
	return amount.update(ctx)
	     | weight.update(ctx);
}


// ----------------------------------------------------------------------------- : PackInstance

PackInstance::PackInstance(const PackType& pack_type, PackGenerator& parent)
	: pack_type(pack_type)
	, parent(parent)
	, requested_copies(0)
	, card_copies(0)
	, expected_copies(0)
{
	// Filter cards
	if (pack_type.filter) {
		FOR_EACH(card, parent.set->cards) {
			Context& ctx = parent.set->getContext(card);
			bool keep = *pack_type.filter.invoke(ctx);
			if (keep) {
				cards.push_back(card);
			}
		}
	}
	// Sum of weights
	if (pack_type.select == SELECT_FIRST) {
		total_weight = cards.empty() ? 0 : 1;
	} else {
		total_weight = cards.size();
	}
	FOR_EACH_CONST(item, pack_type.items) {
		if (pack_type.select == SELECT_PROPORTIONAL || pack_type.select == SELECT_EQUAL_PROPORTIONAL) {
			total_weight += item->weight * parent.get(item->name).total_weight;
		} else if (pack_type.select == SELECT_NONEMPTY || pack_type.select == SELECT_EQUAL_NONEMPTY) {
			if (parent.get(item->name).total_weight > 0) {
				total_weight += item->weight;
			}
		} else if (pack_type.select == SELECT_FIRST) {
			if (total_weight <= 0) {
				total_weight = item->weight;
				break;
			}
		} else {
			total_weight += item->weight;
		}
	}
	// Depth
	depth = 0;
	FOR_EACH_CONST(item, pack_type.items) {
		depth = max(depth, 1 + parent.get(item->name).depth);
	}
}

void PackInstance::expect_copy(double copies) {
	this->expected_copies += copies;
	// propagate
	FOR_EACH_CONST(item, pack_type.items) {
		PackInstance& i = parent.get(item->name);
		if (pack_type.select == SELECT_ALL) {
			i.expect_copy(copies * item->amount);
		} else if (pack_type.select == SELECT_PROPORTIONAL || pack_type.select == SELECT_EQUAL_PROPORTIONAL) {
			i.expect_copy(copies * item->amount * item->weight * i.total_weight / total_weight);
		} else if (pack_type.select == SELECT_NONEMPTY || pack_type.select == SELECT_EQUAL_NONEMPTY) {
			if (i.total_weight > 0) {
				i.expect_copy(copies * item->amount * item->weight / total_weight);
			}
		} else if (pack_type.select == SELECT_FIRST) {
			if (i.total_weight > 0 && cards.empty()) {
				i.expect_copy(copies * item->amount);
				break;
			}
		} else {
			i.expect_copy(copies * item->amount * item->weight / total_weight);
		}
	}
}

void PackInstance::request_copy(size_t copies) {
	requested_copies += copies;
}

/// Random generator with random numbers in a range
template <typename Gen>
struct RandomRange {
	RandomRange(Gen& gen) : gen(gen) {}
	unsigned operator () (unsigned max) { return gen() % max; }
	Gen& gen;
};

struct WeightedItem {
	double weight;
	int count;
	int tiebreaker;
};

struct CompareWeightedItems{
	inline bool operator () (WeightedItem* a, WeightedItem* b) {
		// compare   (a->count+1)/a->weight  <>  (b->count+1)/b->weight
		// prefer the one where this is lower, return true if b is prefered
		double delta = b->weight * (a->count + 1) - a->weight * (b->count + 1);
		if (delta < 0) return false;
		if (delta > 0) return true;
		return b->tiebreaker < a->tiebreaker;
	}
};

/// Distribute 'total' among the weighted items, higher weight items get chosen more often
void weighted_equal_divide(vector<WeightedItem>& items, int total) {
	assert(!items.empty());
	if (items.size() == 1) {
		items.front().count = total;
	} else {
		priority_queue<WeightedItem*,vector<WeightedItem*>,CompareWeightedItems> pq;
		for (size_t i = 0 ; i < items.size() ; ++i) {
			pq.push(&items[i]);
		}
		while (total > 0) {
			// repeatedly pick the item that minimizes, after incrementing count:
			//   max_wi  wi->count/wi->weight
			WeightedItem* wi = pq.top();pq.pop();
			wi->count++;
			total--;
			pq.push(wi);
		}
	}
}

void PackInstance::generate(vector<CardP>* out) {
	card_copies = 0;
	if (requested_copies == 0) return;
	if (pack_type.select == SELECT_ALL) {
		// add all cards
		generate_all(out, requested_copies);

	} else if (pack_type.select == SELECT_REPLACE
	        || pack_type.select == SELECT_PROPORTIONAL
	        || pack_type.select == SELECT_NONEMPTY) {
		// multiple copies
		for (size_t i = 0 ; i < requested_copies ; ++i) {
			generate_one_random(out);
		}

	} else if (pack_type.select == SELECT_NO_REPLACE) {
		if (!pack_type.items.empty()) {
			throw Error(_("'select:no replace' is not yet supported in combination with 'items', only with 'filter'."));
		}
		card_copies += requested_copies;
		// NOTE: there is no way to pick items without replacement
		if (out && !cards.empty()) {
			// to prevent us from being too predictable for small sets, periodically reshuffle
			RandomRange<boost::mt19937> gen_range(parent.gen);
			int max_per_batch = ((int)cards.size() + 1) / 2;
			int rem = (int)requested_copies;
			while (rem > 0) {
				random_shuffle(cards.begin(), cards.end(), gen_range);
				out->insert(out->end(), cards.begin(), cards.begin() + min(rem, max_per_batch));
				rem -= max_per_batch;
			}
		}

	} else if (pack_type.select == SELECT_EQUAL
	        || pack_type.select == SELECT_EQUAL_PROPORTIONAL
	        || pack_type.select == SELECT_EQUAL_NONEMPTY) {
		// equal selection instead of random
		if (requested_copies == 1) {
			// somewhat of a hack to keep things fair: just pick at random
			// otherwise we would end up picking the lowest weight item
			generate_one_random(out);
		} else {
			// 1. the weights of each item, and of the cards
			vector<WeightedItem> weighted_items;
			FOR_EACH_CONST(item, pack_type.items) {
				WeightedItem wi = {0,0,parent.gen()};
				if (pack_type.select == SELECT_EQUAL_PROPORTIONAL) {
					wi.weight = item->weight * parent.get(item->name).total_weight;
				} else if (pack_type.select == SELECT_EQUAL_NONEMPTY) {
					wi.weight = parent.get(item->name).total_weight > 0 ? static_cast<int>(item->weight) : 0;
				} else {
					wi.weight = item->weight;
				}
				weighted_items.push_back(wi);
			}
			WeightedItem wi = {cards.size(),0,parent.gen()};
			weighted_items.push_back(wi);
			// 2. divide the requested_copies among the cards and the items, taking the weights into account
			weighted_equal_divide(weighted_items, (int)requested_copies);
			// 3a. propagate to items
			for (size_t j = 0 ; j < pack_type.items.size() ; ++j) {
				const PackItem& item = *pack_type.items[j];
				PackInstance& i = parent.get(item.name);
				i.request_copy(item.amount * weighted_items[j].count);
			}
			// 3b. pick some cards
			int new_card_copies = weighted_items.back().count;
			card_copies += new_card_copies;
			if (out && !cards.empty()) {
				int div = new_card_copies / (int)cards.size();
				int rem = new_card_copies % (int)cards.size();
				// some copies of all cards
				for (int i = 0 ; i < div ; ++i) {
					out->insert(out->end(), cards.begin(), cards.end());
				}
				// pick the remainder at random
				for (int i = 0 ; i < rem ; ++i) {
					int nr = parent.gen() % cards.size();
					out->push_back(cards.at(nr));
				}
			}
		}

	} else if (pack_type.select == SELECT_FIRST) {
		if (!cards.empty()) {
			// there is a card, pick it
			card_copies += requested_copies;
			if (out) out->insert(out->end(), requested_copies, cards.front());
		} else {
			// pick first nonempty item
			FOR_EACH_CONST(item, pack_type.items) {
				PackInstance& i = parent.get(item->name);
				if (i.total_weight > 0) {
					i.request_copy(requested_copies * item->amount);
					break;
				}
			}
		}
	}
	requested_copies = 0;
}

void PackInstance::generate_all(vector<CardP>* out, size_t copies) {
	card_copies += copies * cards.size();
	if (out) {
		for (size_t i = 0 ; i < copies ; ++i) {
			out->insert(out->end(), cards.begin(), cards.end());
		}
	}
	// and all items
	FOR_EACH_CONST(item, pack_type.items) {
		PackInstance& i = parent.get(item->name);
		i.request_copy(copies * item->amount);
	}
}

void PackInstance::generate_one_random(vector<CardP>* out) {
	double r = parent.gen() * total_weight / parent.gen.max();
	if (r < cards.size()) {
		// pick a card
		card_copies++;
		if (out) {
			int i = (int)r;
			out->push_back(cards[i]);
		}
	} else {
		// pick an item
		r -= cards.size();
		FOR_EACH_CONST(item, pack_type.items) {
			PackInstance& i = parent.get(item->name);
			if (pack_type.select == SELECT_PROPORTIONAL || pack_type.select == SELECT_EQUAL_PROPORTIONAL) {
				r -= item->weight * i.total_weight;
			} else if (pack_type.select == SELECT_NONEMPTY || pack_type.select == SELECT_EQUAL_NONEMPTY) {
				if (i.total_weight > 0) r -= item->weight;
			} else {
				r -= item->weight;
			}
			// have we reached the item we were looking for?
			if (r < 0) {
				i.request_copy(item->amount);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------------- : PackGenerator

void PackGenerator::reset(const SetP& set, int seed) {
	this->set = set;
	gen.seed((unsigned)seed);
	max_depth = 0;
	instances.clear();
}
void PackGenerator::reset(int seed) {
	gen.seed((unsigned)seed);
}

PackInstance& PackGenerator::get(const String& name) {
	PackInstanceP& instance = instances[name];
	if (instance) {
		return *instance;
	} else {
		FOR_EACH_CONST(type, set->pack_types) {
			if (type->name == name) {
				instance = PackInstanceP(new PackInstance(*type,*this));
				max_depth = max(max_depth, instance->get_depth());
				return *instance;
			}
		}
		FOR_EACH_CONST(type, set->game->pack_types) {
			if (type->name == name) {
				instance = PackInstanceP(new PackInstance(*type,*this));
				max_depth = max(max_depth, instance->get_depth());
				return *instance;
			}
		}
		throw Error(_ERROR_1_("pack type not found",name));
	}
}
PackInstance& PackGenerator::get(const PackTypeP& type) {
	return get(type->name);
}

void PackGenerator::generate(vector<CardP>& out) {
	if (!set) return;
	// We generate from depth max_depth to 0
	// instances can refer to other instances of lower depth, and generate
	// can change the number of copies of those lower depth instances
	for (int depth = max_depth ; depth >= 0 ; --depth) {
		// in game file order
		FOR_EACH_CONST(type, set->game->pack_types) {
			PackInstance& i = get(type);
			if (i.get_depth() == depth) {
				i.generate(&out);
			}
		}
		// ...and then set file order
		FOR_EACH_CONST(type, set->pack_types) {
			PackInstance& i = get(type);
			if (i.get_depth() == depth) {
				i.generate(&out);
			}
		}
	}
}

void PackGenerator::update_card_counts() {
	if (!set) return;
	// update card_counts by using generate()
	for (int depth = max_depth ; depth >= 0 ; --depth) {
		FOR_EACH_CONST(i,instances) {
			if (i.second->get_depth() == depth) {
				i.second->generate(nullptr);
			}
		}
	}
}

#endif
