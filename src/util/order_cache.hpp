//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ORDER_CACHE
#define HEADER_UTIL_ORDER_CACHE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : OrderCache

/// Object that cashes an ordered version of a list of items, for finding the position of objects
/** Can be used as a map "void* -> int" for finding the position of an object */
template <typename T>
class OrderCache {
  public:
	/// Initialize the order cache, ordering the keys by their string values from the other vector
	/** @pre keys.size() == values.size() */
	OrderCache(const vector<T>& keys, const vector<String>& values);
	
	/// Find the position of the given key in the cache, returns -1 if not found
	int find(const T& key) const;
	
  private:
	struct CompareKeys;
	struct CompareValues;
	typedef pair<void*,int> KV;
	vector<KV> positions;
};

// ----------------------------------------------------------------------------- : Implementation

template <typename T>
struct OrderCache<T>::CompareKeys {
	inline bool operator () (const KV& a, void*     b) { return a.first < b; }
	inline bool operator () (const KV& a, const KV& b) { return a.first < b.first; }
	inline bool operator () (void*     a, const KV& b) { return a       < b.first; }
};

template <typename T>
struct OrderCache<T>::CompareValues {
	const vector<String>& values;
	CompareValues(const vector<String>& values) : values(values) {}
	
	inline bool operator () (const KV& a, const KV& b) {
		return values[a.second] < values[b.second];
	}
};

template <typename T>
OrderCache<T>::OrderCache(const vector<T>& keys, const vector<String>& values) {
	assert(keys.size() == values.size());
	// initialize positions, use pos to point back to the values vector
	positions.reserve(keys.size());
	int i = 0;
	for (vector<T>::const_iterator it = keys.begin() ; it != keys.end() ; ++it, ++i) {
		positions.push_back(KV(&**it, i));
	}
	// sort the KVs by the values
	sort(positions.begin(), positions.end(), CompareValues(values));
	// update positions, to point to sorted list
	i = 0;
	for (vector<KV>::iterator it = positions.begin() ; it != positions.end() ; ++it, ++i) {
		it->second = i;
	}
	// sort the KVs by the keys
	sort(positions.begin(), positions.end(), CompareKeys());
}

template <typename T>
int OrderCache<T>::find(const T& key) const {
	vector<KV>::const_iterator it = lower_bound(positions.begin(), positions.end(), &*key, CompareKeys());
	if (it == positions.end() || it->first != &*key) return -1;
	return it->second;
}

// ----------------------------------------------------------------------------- : EOF
#endif
