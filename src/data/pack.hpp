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

DECLARE_POINTER_TYPE(PackItemRef);
DECLARE_POINTER_TYPE(Card);
class Set;

// ----------------------------------------------------------------------------- : PackType

/// A card pack description for playtesting
class PackType : public IntrusivePtrBase<PackType> {
  public:
	PackType();
	
	String               name;    ///< Name of this pack
	Scriptable<bool>     enabled; ///< Is this pack enabled?
	vector<PackItemRefP> items;   ///< Cards in this pack
	
	/// Generate a random pack of cards, add them to out
	void generate(Set& set, vector<CardP>& out) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackItemRef

class PackItemRef : public IntrusivePtrBase<PackItemRef> {
  public:
	PackItemRef();
	
	String          name;	///< Name of this type of cards
	Scriptable<int> amount;	///< Number of cards of this type
	
	/// Update scripts, returns true if there is a change
	bool update(Context& ctx);
	
	/// Generate random cards, add them to out
	void generate(Set& set, vector<CardP>& out) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackItem

/// A card type description for playtesting
class PackItem : public IntrusivePtrBase<PackItem> {
  public:
	String          name;	///< Name of this type of cards
	OptionalScript  filter;	///< Filter to select this type of cards
	
	/// Generate random cards, add them to out
	void generate(Set& set, vector<CardP>& out) const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
