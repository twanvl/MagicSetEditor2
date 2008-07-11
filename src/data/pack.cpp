//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/pack.hpp>

// ----------------------------------------------------------------------------- : PackType

PackType::PackType()
	: enabled(true)
{}

IMPLEMENT_REFLECTION(PackType) {
	REFLECT(name);
	REFLECT(enabled);
	REFLECT(items);
}

void PackType::generate(Set& set, vector<CardP>& out) const {
	//%FOR_EACH(card_type, card_types) {
	//%	card_type->generate(set,out);
	//%}
}

// ----------------------------------------------------------------------------- : PackItemRef

PackItemRef::PackItemRef()
	: amount(1)
{}

IMPLEMENT_REFLECTION(PackItemRef) {
	REFLECT(name);
	REFLECT(amount);
}

bool PackItemRef::update(Context& ctx) {
	return amount.update(ctx);
}

// ----------------------------------------------------------------------------- : PackItem

IMPLEMENT_REFLECTION(PackItem) {
	REFLECT(name);
	REFLECT(filter);
}

void PackItem::generate(Set& set, vector<CardP>& out) const {
	//%Context& ctx = set.getContext();
	//%amount.update(ctx);
	//%FOR_EACH(card_type, card_types) {
	//%	card_type->generate(set,out);
	//%}
}
