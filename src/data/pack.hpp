//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_PACK
#define HEADER_DATA_PACK

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <script/scriptable.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/logic/tribool.hpp>

#define USE_NEW_PACK_SYSTEM 1

#if !USE_NEW_PACK_SYSTEM
// =================================================================================================== OLD

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




#else
// =================================================================================================== NEW

DECLARE_POINTER_TYPE(PackType);
DECLARE_POINTER_TYPE(PackItem);
DECLARE_POINTER_TYPE(PackInstance);
DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Set);
class PackGenerator;

// ----------------------------------------------------------------------------- : PackType

enum PackSelectType
{	SELECT_AUTO
,	SELECT_ALL
,	SELECT_NO_REPLACE
,	SELECT_REPLACE
,	SELECT_PROPORTIONAL
,	SELECT_NONEMPTY
,	SELECT_EQUAL
,	SELECT_EQUAL_PROPORTIONAL
,	SELECT_EQUAL_NONEMPTY
,	SELECT_FIRST
};

/// A card pack description for playtesting
class PackType : public IntrusivePtrBase<PackType> {
  public:
	PackType();
	
	String            name;       ///< Name of this pack
	Scriptable<bool>  enabled;    ///< Is this pack enabled?
	tribool           selectable; ///< Is this pack listed in the UI?
	tribool           summary;    ///< Should the total be listed for this type?
	PackSelectType    select;     ///< What cards/items to select
	OptionalScript    filter;     ///< Filter to select this type of cards
	vector<PackItemP> items;      ///< Subpacks in this pack
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
  private:
	DECLARE_REFLECTION();
};

/// An item in a PackType
class PackItem : public IntrusivePtrBase<PackItem> {
  public:
	PackItem();
	PackItem(const String& name, int amount);
	
	String             name;         ///< Name of the pack to select cards from
	Scriptable<int>    amount;       ///< Number of cards of this type
	Scriptable<double> weight;       ///< Relative probability of picking this item
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
  private:
	DECLARE_REFLECTION();
};

inline String type_name(const PackType&) {
	return _TYPE_("pack");
}

// ----------------------------------------------------------------------------- : Generating / counting

// A PackType that is instantiated for a particular Set,
// i.e. we now know the actual cards
class PackInstance : public IntrusivePtrBase<PackInstance> {
  public:
	PackInstance(const PackType& pack_type, PackGenerator& parent);
	
	/// Expect to pick this many copies from this pack, updates expected_copies
	void expect_copy(double copies = 1);
	/// Request some copies of this pack
	void request_copy(size_t copies = 1);
	
	/// Generate cards if depth == at_depth
	/** Some cards are (optionally) added to out and card_copies
	  * And also the copies of referenced items might be incremented
	  *
	  * Resets the count of this instance to 0 */
	void generate(vector<CardP>* out);
	
	inline int    get_depth()           const { return depth; }
	inline bool   has_cards()           const { return !cards.empty(); }
	inline size_t get_card_copies()     const { return card_copies; }
	inline double get_expected_copies() const { return expected_copies; }
	
  private:
	const PackType& pack_type;
	PackGenerator&  parent;
	int             depth;             //< 0 = no items, otherwise 1+max depth of items refered to
	vector<CardP>   cards;             //< All cards that pass the filter
	double          total_weight;      //< Sum of item and card weights
	size_t          requested_copies;  //< The requested number of copies of this pack
	size_t          card_copies;       //< The number of cards that were chosen to come from this pack
	double          expected_copies;
	
	/// Generate some copies of all cards and items
	void generate_all(vector<CardP>* out, size_t copies);
	/// Generate one card/item chosen at random (using the select type)
	void generate_one_random(vector<CardP>* out);
};

class PackGenerator {
  public:
	/// Reset the generator, possibly switching the set or reseeding
	void reset(const SetP& set, int seed);
	/// Reset the generator, but not the set
	void reset(int seed);
	
	/// Find the PackInstance for the PackType with the given name
	PackInstance& get(const String& name);
	PackInstance& get(const PackTypeP& type);
	
	/// Generate all cards, resets copies
	void generate(vector<CardP>& out);
	/// Update all card_copies counters, resets copies
	void update_card_counts();
	
	// only for PackInstance
	SetP set;           ///< The set
	boost::mt19937 gen; ///< Random generator
  private:
	/// Details for each PackType
	map<String,PackInstanceP> instances;
	int max_depth;
};

// ----------------------------------------------------------------------------- : EOF
#endif
#endif
