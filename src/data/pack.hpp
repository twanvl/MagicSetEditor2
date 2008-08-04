//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_PACK
#define HEADER_DATA_PACK

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <script/scriptable.hpp>
#include <boost/random/mersenne_twister.hpp>

DECLARE_POINTER_TYPE(PackItemRef);
DECLARE_POINTER_TYPE(Card);
class Set;
class PackItemCache;

// ----------------------------------------------------------------------------- : PackType

/// A card pack description for playtesting
class PackType : public IntrusivePtrBase<PackType> {
  public:
	PackType();
	
	String               name;    ///< Name of this pack
	Scriptable<bool>     enabled; ///< Is this pack enabled?
	vector<PackItemRefP> items;   ///< Cards in this pack
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
	/// Generate a random pack of cards, add them to out
	void generate(PackItemCache& packs, boost::mt19937& gen, vector<CardP>& out) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackItemRef

enum PackRefType
{	PACK_REF_REPLACE
,	PACK_REF_NO_REPLACE
,	PACK_REF_CYCLIC
};

class PackItemRef : public IntrusivePtrBase<PackItemRef> {
  public:
	PackItemRef();
	
	String          name;	///< Name of this type of cards
	Scriptable<int> amount;	///< Number of cards of this type
	PackRefType     type; 
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
	/// Generate random cards, add them to out
	void generate(PackItemCache& packs, boost::mt19937& gen, vector<CardP>& out) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackItem

/// A card type description for playtesting
class PackItem : public IntrusivePtrBase<PackItem> {
  public:
	String         name;	///< Name of this type of cards
	OptionalScript filter;	///< Filter to select this type of cards
	
	/// Select *all* cards matching the filter
	void generate(Set& set, vector<CardP>& out) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackItemCache

class PackItemCache {
  public:
	PackItemCache(Set& set);
	
	/// The cards for a given PackItem
	vector<CardP>& cardsFor(const String& name);
	
  private:
	Set& set;
	/// Cards for each PackItem
	typedef shared_ptr<vector<CardP> > Cards;
	map<String,Cards> item_cards;
};

// ----------------------------------------------------------------------------- : EOF
#endif
