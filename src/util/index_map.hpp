//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_INDEX_MAP
#define HEADER_UTIL_INDEX_MAP

// ----------------------------------------------------------------------------- : Includes

#include <vector>

// ----------------------------------------------------------------------------- : IndexMap

/// A kind of map of K->V, with the following properties:
/**  - K must have a unique member ->index of type UInt
 *   - There must exist a function initObject(K, V&)
 *     that stores a new V object for a given key in the reference
 *   - O(1) inserts and lookups
 */
template <typename Key, typename Value>
class IndexMap : private vector<Value> {
  public:
	using vector<Value>::empty;
	using vector<Value>::size;
	using vector<Value>::iterator;
	using vector<Value>::begin;
	using vector<Value>::end;
	
	/// Initialize this map with default values given a list of keys, has no effect if !empty()
	/** Requires a function
	 *    void initObject(Key, Value&)
	 */
	void init(const vector<Key>& keys) {
		if (!this->empty()) return;
		this->reserve(keys.size());
		FOR_EACH_CONST(key, keys) {
			assert(key);
			if (key->index >= this->size()) this->resize(key->index + 1);
			initObject(key, (*this)[key->index]);
		}
	}
	
	/// Retrieve a value given its key
	inline Value operator [] (const Key& key) {
		assert(key);
		assert(this->size() > key->index);
		return at(key->index);
	}

	/// Is a value contained in this index map?
	/// requires a function Key Value::getKey()
	inline bool contains(const Value& value) const {
		assert(value);
		size_t index = value->getKey()->index;
		return index < this.size() && (*this)[index] == value
	}

	/// Is a key in the domain of this index map?
	/// requires a function Key Value::getKey()
	inline bool containsKey(const Key& key) const {
		assert(key);
		return key->index < this.size() && (*this)[key->index]->getKey() == key
	}
	
  private:
	using vector<Value>::operator [];
};


// ----------------------------------------------------------------------------- : EOF
#endif
