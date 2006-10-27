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

/// A kind of map of Key->Value, with the following properties:
/**  - K must have a unique member ->index of type UInt
 *   - There must exist a function void init_object(Key, Value&)
 *     that stores a new V object for a given key in the reference
 *   - There must exist a function Key get_key(Value)
 *     that returns a key for a given value
 *   - For reflection there must exist a function String get_key_name(Value)
 *     that returns the key in string form
 *   - O(1) inserts and lookups
 *
 *  The 'map' is actually just a vector of values, each key has an index
 *  which is used for the vector.
 *  Values know their keys, so there is no need to store them separately.
 */
template <typename Key, typename Value>
class IndexMap : private vector<Value> {
  public:
	using vector<Value>::empty;
	using vector<Value>::size;
	using vector<Value>::iterator;
	using vector<Value>::const_iterator;
	using vector<Value>::reference;
	using vector<Value>::const_reference;
	using vector<Value>::begin;
	using vector<Value>::end;
	
	/// Initialize this map with default values given a list of keys, has no effect if !empty()
	void init(const vector<Key>& keys) {
		if (!this->empty()) return;
		this->reserve(keys.size());
		for(vector<Key>::const_iterator it = keys.begin() ; it != keys.end() ; ++it) {
			const Key& key = *it;
			assert(key);
			if (key->index >= this->size()) this->resize(key->index + 1);
			init_object(key, (*this)[key->index]);
		}
	}
	
	/// Retrieve a value given its key
	inline Value operator [] (const Key& key) {
		assert(key);
		assert(this->size() > key->index);
		return at(key->index);
	}
	
	/// Is a value contained in this index map?
	inline bool contains(const Value& value) const {
		assert(value);
		size_t index = get_key(value)->index;
		return index < this->size() && (*this)[index] == value;
	}
	
	/// Is a key in the domain of this index map?
	inline bool containsKey(const Key& key) const {
		assert(key);
		return key->index < this.size() && get_key((*this)[key->index]) == key;
	}
	
	/// Find a value given the key name, return an iterator
	template <typename Name>
	const_iterator find(const Name& key) const {
		for(vector<Value>::const_iterator it = begin() ; it != end() ; ++it) {
			if (get_key_name(*it) == key) return it;
		}
		return end();
	}
	
  private:
	using vector<Value>::operator [];
};


// ----------------------------------------------------------------------------- : EOF
#endif
