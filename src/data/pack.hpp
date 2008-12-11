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

#define USE_NEW_PACK_SYSTEM 0

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

DECLARE_POINTER_TYPE(PackItem);
DECLARE_POINTER_TYPE(Card);
class Set;

// ----------------------------------------------------------------------------- : PackType

enum OneMany
{	SELECT_ONE_OR_EMPTY
,	SELECT_ONE
,	SELECT_FIRST
,	SELECT_ALL
};

/// A card pack description for playtesting
class PackType : public IntrusivePtrBase<PackType> {
  public:
	PackType();
	
	String            name;       ///< Name of this pack
	Scriptable<bool>  enabled;    ///< Is this pack enabled?
	bool              selectable; ///< Is this pack listed in the UI?
	bool              summary;    ///< Should the total be listed for this type?
	OneMany           select;     ///< Select one or many?
	OptionalScript    cards;      ///< Script to select this type of cards (there are no items)
	OptionalScript    filter;     ///< Filter to select this type of cards
	vector<PackItemP> items;      ///< Subpacks in this pack
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackItem

enum PackSelectType
{	PACK_REF_INHERIT
,	PACK_REF_REPLACE
,	PACK_REF_NO_REPLACE
,	PACK_REF_CYCLIC
};

/// An item in a PackType
class PackItem : public IntrusivePtrBase<PackItem> {
  public:
	PackItem();
	
	String             pack;    ///< Name of the pack to select cards from
	Scriptable<int>    amount;  ///< Number of cards of this type
	Scriptable<double> weight;  ///< Relative probability of picking this item
	PackSelectType     type;
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Generating / counting

// --------------------------------------------------- : PackItemCache

class PackItemCache {
  public:
	PackItemCache(Set& set) : set(set) {}
	
	/// Look up a pack type by name
	const PackType& pack(const String& name);
	
  protected:
	Set& set;
	
	/// The cards for a given PackItem
	ScriptValueP cardsFor(const ScriptValueP& cards_script);	
	
  private:
	/// Lookup PackTypes by name
	//%% 
	/// Cards for each PackType
	map<ScriptValueP,ScriptValueP> item_cards;
};

// --------------------------------------------------- : Counting expected cards

/// Class for determining the *expected* number of cards from each type
class PackItemCounter : PackItemCache {
  public:
	PackItemCounter(Set& set, map<const PackType*,double>& counts)
		: PackItemCache(set), counts(counts)
	{}
	
	/// Add a number of copies of the PackType to the counts, recurse into child items
	void addCountRecursive(const PackType& pack, double copies);
	void addCountRecursive(const PackItem& item, double copies);
	
	/// The probability that the given pack is non-empty
	double probabilityNonEmpty(const PackType& pack);
	double probabilityNonEmpty(const PackItem& item);
	
	/// The counts will be stored here
	map<const PackType*,double>& counts;
	
  private:
	/// The probability that a pack type is empty (cache)
	//%map<const PackItem*,double> probability_empty;
};

// --------------------------------------------------- : PackItemCounter

/// Class for generating card packs
class PackItemGenerator : PackItemCache {
  public:
	PackItemGenerator(Set& set, vector<CardP>& cards, boost::mt19937& gen)
		: PackItemCache(set), out(cards), gen(gen)
	{}
	
	/// Generate a pack, adding it to cards
	void generate(const PackType& pack);
	
	/// Number of cards of a type
	typedef map<pair<ScriptValueP,PackSelectType>,int> OfTypeCount;
	
	/// Determine what *types* of cards to pick (store in out)
	/** Does NOT add cards yet.
	 *  Returns true if non-empty.
	 */
	bool generateCount(const PackType& pack, int copies, PackSelectType type, OfTypeCount& out);
	bool generateCount(const PackItem& item, int copies, PackSelectType type, OfTypeCount& out);
	bool generateSingleCount(const PackType& pack, PackSelectType type, OfTypeCount& out);
	
	/// Pick cards from a list
	void pickCards(const ScriptValueP& cards, PackSelectType type, int amount);
	
	/// The cards will be stored here
	vector<CardP>& out;
	
	/// Random generator
	boost::mt19937& gen;
};

// ----------------------------------------------------------------------------- : EOF
#endif
#endif
