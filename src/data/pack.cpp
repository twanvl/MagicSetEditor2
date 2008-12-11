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

// ----------------------------------------------------------------------------- : PackType

PackType::PackType()
	: enabled(true)
	, selectable(true)
	, summary(true)
	, select(SELECT_ALL)
{}

IMPLEMENT_REFLECTION_ENUM(OneMany) {
	VALUE_N("all",         SELECT_ALL);
	VALUE_N("at most one", SELECT_ONE_OR_EMPTY);
	VALUE_N("one",         SELECT_ONE);
	VALUE_N("first",       SELECT_FIRST);
}

IMPLEMENT_REFLECTION(PackType) {
	REFLECT(name);
	REFLECT(enabled);
	REFLECT(selectable);
	REFLECT(summary);
	REFLECT(select);
	REFLECT(cards);
	REFLECT(items);
}

bool PackType::update(Context& ctx) {
	bool change = enabled.update(ctx);
	FOR_EACH(item, items) {
		change |= item->update(ctx);
	}
	return change;
}

// ----------------------------------------------------------------------------- : PackItem

PackItem::PackItem()
	: amount(1)
	, type(PACK_REF_INHERIT)
{}

IMPLEMENT_REFLECTION_ENUM(PackSelectType) {
	VALUE_N("inherit",    PACK_REF_INHERIT);
	VALUE_N("replace",    PACK_REF_REPLACE);
	VALUE_N("no replace", PACK_REF_NO_REPLACE);
	VALUE_N("cyclic",     PACK_REF_CYCLIC);
}

IMPLEMENT_REFLECTION(PackItem) {
	REFLECT(pack);
	REFLECT(amount);
	REFLECT(type);
}

bool PackItem::update(Context& ctx) {
	return amount.update(ctx);
}

// ----------------------------------------------------------------------------- : PackItemCache

ScriptValueP PackItemCache::cardsFor(const ScriptValueP& generate) {
	// lookup name
	ScriptValueP& value = item_cards[generate];
	if (!value) {
		value = generate->eval(set.getContext());
	}
	return value;
}

const PackType& PackItemCache::pack(const String& name) {
	// not used before, generate list and cache
	FOR_EACH(pack, set.game->pack_types) {
		if (pack->name == name) {
			return *pack;
		}
	}
	// not found
	throw Error(_ERROR_1_("pack type not found", name));
}

// ----------------------------------------------------------------------------- : Counting expected cards

double PackItemCounter::probabilityNonEmpty(const PackType& pack) {
	// TODO: cache?
	if (pack.cards) {
		return cardsFor(pack.cards.getScriptP())->itemCount();
	} else if (pack.select == SELECT_ONE_OR_EMPTY) {
		// weighted avarage
		double p = 0.0;
		double total_weight = 0.0;
		FOR_EACH_CONST(i,pack.items) {
			p            += i->weight * probabilityNonEmpty(*i);
			total_weight += i->weight;
		}
		return p / total_weight;
	} else { // SELECT_ONE, SELECT_FIRST, SELECT_ALL
		// disjunction
		double p = 0.0;
		FOR_EACH_CONST(i,pack.items) {
			// either already non-empty, or all previous items were empty so pick this one
			p += (1-p) * probabilityNonEmpty(*i);
			if (p >= 1 - 1e-6) return 1.0;
		}
		return p;
	}
}
double PackItemCounter::probabilityNonEmpty(const PackItem& item) {
	return item.amount <= 0 ? 0 : probabilityNonEmpty(pack(item.pack));
}

void PackItemCounter::addCountRecursive(const PackType& pack, double copies) {
	// add
	counts[&pack] += copies * probabilityNonEmpty(pack);
	// recurse
	if (pack.cards) {
		// done
	} else if (pack.select == SELECT_FIRST) {
		double p = 1;
		FOR_EACH_CONST(i, pack.items) {
			addCountRecursive(*i, p * copies);
			p *= 1 - probabilityNonEmpty(*i);
			if (p < 1e-6) return;
		}
	} else if (pack.select == SELECT_ONE_OR_EMPTY || pack.select == SELECT_ONE) {
		double total_weight = 0.0;
		FOR_EACH_CONST(i, pack.items) {
			total_weight += i->weight * (pack.select == SELECT_ONE ? probabilityNonEmpty(*i) : 1.0);
		}
		FOR_EACH_CONST(i, pack.items) {
			addCountRecursive(*i, copies * i->weight / total_weight);
		}
	} else if (pack.select == SELECT_ALL) {
		FOR_EACH_CONST(i, pack.items) {
			addCountRecursive(*i, copies);
		}
	} else {
		throw InternalError(_("unknown OneMany value"));
	}
}

void PackItemCounter::addCountRecursive(const PackItem& item, double copies) {
	addCountRecursive(pack(item.pack), item.amount * copies);
}


// ----------------------------------------------------------------------------- : Generating

DECLARE_TYPEOF(PackItemGenerator::OfTypeCount);

/// Random generator with random numbers in a range
template <typename Gen>
struct RandomRange {
	RandomRange(Gen& gen) : gen(gen) {}
	unsigned operator () (unsigned max) { return gen() % max; }
	Gen& gen;
};

bool PackItemGenerator::generateCount(const PackType& pack, int copies, PackSelectType type, OfTypeCount& out) {
	if (copies <= 0) return false;
	bool non_empty = false;
	if (pack.cards) {
		ScriptValueP the_cards = cardsFor(pack.cards.getScriptP());
		non_empty = the_cards->itemCount() > 0;
		if (non_empty) {
			out[make_pair(the_cards,type)] += copies;
		}
	} else if (pack.select == SELECT_ALL) {
		// just generate all
		FOR_EACH_CONST(i, pack.items) {
			non_empty |= generateCount(*i, 1, type, out);
		}
	} else {
		// generate each copy separately
		for (int j = 0 ; j < copies ; ++j) {
			non_empty |= generateSingleCount(pack, type, out);
		}
	}
	return non_empty;
}

bool PackItemGenerator::generateSingleCount(const PackType& pack, PackSelectType type, OfTypeCount& out) {
	if (pack.select == SELECT_ONE_OR_EMPTY) {
		// pick a random item by weight
		double total_weight = 0.0;
		FOR_EACH_CONST(i, pack.items) {
			total_weight += i->weight;
		}
		double choice = gen() * total_weight / gen.max();
		FOR_EACH_CONST(i, pack.items) {
			if ((choice -= i->weight) <= 0) {
				// pick this one
				return generateCount(*i, 1, type, out);
			}
		}
	} else if (pack.select == SELECT_ONE) {
		// pick a random item by weight that is not empty
		UInt possible = 0; // bitmask
		double total_weight = 0.0;
		for (size_t i = 0 ; i < pack.items.size() ; ++i) {
			total_weight += pack.items[i]->weight;
			possible |= 1 << i;
		}
		while (possible) {
			// try to make a choice we have not made before
			int choice = gen() * total_weight / gen.max();
			for (size_t i = 0 ; i < pack.items.size() ; ++i) {
				const PackItem& item = *pack.items[i];
				if (!(possible & (1<<i))) continue; // already tried this item?
				if ((choice -= item.weight) <= 0) {
					bool non_empty = generateCount(item, 1, type, out);
					if (non_empty) {
						// found a non-empty choice, done
						return true;
					} else {
						// try again, exclude this item
						possible &= ~(1 << i);
						total_weight -= item.weight;
						break;
					}
				}
			}
		}
	} else if (pack.select == SELECT_FIRST) {
		// pick the first one that is not empty
		FOR_EACH_CONST(i, pack.items) {
			bool non_empty = generateCount(*i, 1, type, out);
			if (non_empty) return true;
		}
	} else {
		throw InternalError(_("unknown OneMany value"));
	}
	return false;
}
bool PackItemGenerator::generateCount(const PackItem& item, int copies, PackSelectType type, OfTypeCount& out) {
	return generateCount(pack(item.pack), copies * item.amount, item.type == PACK_REF_INHERIT ? type : item.type, out);
}

void PackItemGenerator::generate(const PackType& pack) {
	// first determine how many cards of each basic type we need
	OfTypeCount counts;
	generateCount(pack, 1, PACK_REF_NO_REPLACE, counts);
	// now select these cards
	FOR_EACH(c, counts) {
		pickCards(c.first.first, c.first.second, c.second);
	}
}
void PackItemGenerator::pickCards(const ScriptValueP& cards, PackSelectType type, int amount) {
	// generate 'amount' cards and add them to out
	int cards_size = cards->itemCount();
	if (cards_size <= 0) return;
	RandomRange<boost::mt19937> gen_range(gen);
	if (type == PACK_REF_REPLACE) {
		// amount random numbers
		for (int i = 0 ; i < amount ; ++i) {
			int index = gen_range(cards_size);
			out.push_back(from_script<CardP>(cards->getIndex(index)));
		}
	} else if (type == PACK_REF_NO_REPLACE) {
		// random shuffle
		// to prevent us from being too predictable for small sets, periodically reshuffle
		int max_per_batch = (cards_size + 1) / 2;
		while (amount > 0) {
			int to_add = min(amount, max_per_batch);
			size_t old_out_size = out.size();
			// add all to output temporarily
			ScriptValueP it = cards->makeIterator(cards);
			while (ScriptValueP card = it->next()) {
				out.push_back(from_script<CardP>(card));
			}
			// shuffle and keep only the first to_add
			random_shuffle(out.begin() + old_out_size, out.end(), gen_range);
			out.resize(old_out_size + to_add);
			amount -= to_add;
		}
	} else if (type == PACK_REF_CYCLIC) {
		// multiple copies
		int copies = amount / cards_size;
		ScriptValueP it = cards->makeIterator(cards);
		while (ScriptValueP card = it->next()) {
			out.insert(out.end(), copies, from_script<CardP>(card));
		}
		amount -= copies * cards_size;
		// if amount is not a multiple of the number of cards, pick the rest at random
		for (int i = 0 ; i < amount ; ++i) {
			int index = gen_range(cards_size);
			out.push_back(from_script<CardP>(cards->getIndex(index)));
		}
	}
}



/*//%
// ----------------------------------------------------------------------------- : PackItem

void PackItemCounter::count(const String& name, double amount) {
	map<PackItemRef*,double>::iterator it = sizes.find(name);
	if (it != sizes.end()) return it->second;
	
	size *= amount * probability;
	return sizes[&item] = size;
}
void PackItemCounter::count(const PackType& pack, int copies) {
	double size = 0;
	if (pack.select = ONE) {
		double total_size;
		FOR_EACH(item, pack.items) {
			double item_size = size(item);
			if (item->type == OTHER_IF_EMPTY) {
				// is it empty?
				total_size += item_size > 0 ? 1 : 0;
				max_size   += 1;
			} else if (item->type == WEIGHTED) {
				total_size += item_size;
				max_size   += item_size;
			} else {
				total_size += 1;
				max_size   += 1;
			}
		}
		size += total_size / max_size;
		// count
		FOR_EACH(item, pack.items) {
			double item_size = size(item);
			if (item_size > 0 && !item->cards) {
				if (item->type == OTHER_IF_EMPTY) {
					// is it empty?
					total_size += item_size > 0 ? 1 : 0;
					max_size   += 1;
				} else if (item->type == WEIGHTED) {
					total_size += item_size;
					max_size   += item_size;
				} else {
					total_size += 1;
					max_size   += 1;
				}
			}
		}
	} else { // MANY
		
	}
	amounts[pack.name] += amount;
}
double PackItemCounter::size(const String& name) {
	map<PackItemRef*,double>::iterator it = sizes.find(name);
	if (it != sizes.end()) return it->second;
	double the_size = 0;
	FOR_EACH() {
		the_size += size(item);
	}
	return sizes[&item] = the_size;
}
double PackItemCounter::size(const PackItemRef& item) {
	String the_name;
	double the_size;
	if (cards) {
		the_size = cards.invokeOn(ctx)->itemCount() > 0 ? 1 : 0;
	} else {
		the_size = size(name);
		the_name = name;
	}
	if (the_size == 0 && !if_empty.empty())
		the_name = if_empty;
		the_size = size(if_empty);
	}
	if (!the_name.empty()) {
		counts[the_name] += amount * probability;
	}
	return the_size * amount * probability;
}


void PackItemCounter::count(const PackType& pack, int copies, type) {
	if (pack.cards) {
		// 
		cards =..
		if (!cards.empty()) {
			
			return true;
		} else {
			return false;
		}
	} else {
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
{}*/

#endif
