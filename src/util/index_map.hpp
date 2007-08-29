//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_INDEX_MAP
#define HEADER_UTIL_INDEX_MAP

// ----------------------------------------------------------------------------- : Includes

#include <vector>
#include <map>
#include <util/string.hpp>

// ----------------------------------------------------------------------------- : IndexMap

/// A kind of map of Key->Value, with the following properties:
/**  - K must have a unique member ->index of type UInt
 *   - There must exist a function void init_object(Key, Value&)
 *     that stores a new V object for a given key in the reference
 *     if the value is already set the function should do nothing
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
	using vector<Value>::clear;
	using vector<Value>::at; // for using numeric indices directly
	
	/// Initialize this map with default values given a list of keys
	/** has no effect if already initialized with the given keys */
	bool init(const vector<Key>& keys) {
		if (!this->empty() && (keys.empty() || get_key(this->front()) != keys.front())) {
			// switch to different keys
			clear();
		}
		if (this->size() == keys.size()) return false;
		this->reserve(keys.size());
		for(typename vector<Key>::const_iterator it = keys.begin() ; it != keys.end() ; ++it) {
			const Key& key = *it;
			assert(key);
			if (key->index >= this->size()) this->resize(key->index + 1);
			init_object(key, (*this)[key->index]);
		}
		return true;
	}
	/// Initialize this map with cloned values from another list
	void cloneFrom(const IndexMap<Key,Value>& values) {
		if (this->size() == values.size()) return;
		this->reserve(values.size());
		for(size_t index = size() ; index < values.size() ; ++index) {
			const Value& value = values[index];
			push_back(value ? value->clone() : value);
		}
	}
	/// Change this map by adding an additional key and value
	void add(const Key& key, const Value& value) {
		assert(get_key(value) == key);
		if (key->index >= this->size()) this->resize(key->index + 1);
		(*this)[key->index] = value;
	}
	
	/// Retrieve a value given its key
	inline Value operator [] (const Key& key) {
		assert(key);
		assert(this->size() > key->index);
		return at(key->index);
	}
	/// Retrieve a value given its key, if it matches
	inline Value tryGet (const Key& key) {
		assert(key);
		if (this->size() <= key->index) return Value();
		Value v = at(key->index);
		if (get_key(v) != key) return Value();
		return v;
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
		return key->index < this->size() && get_key((*this)[key->index]) == key;
	}
	
	/// Find a value given the key name, return an iterator
	template <typename Name>
	typename vector<Value>::const_iterator find(const Name& key) const {
		for(typename vector<Value>::const_iterator it = begin() ; it != end() ; ++it) {
			if (get_key_name(*it) == key) return it;
		}
		return end();
	}
	
	inline void swap(IndexMap& b) {
		vector<Value>::swap(b);
	}
	
  private:
	using vector<Value>::operator [];
};

template <typename Key, typename Value>
inline void swap(IndexMap<Key,Value>& a, IndexMap<Key,Value>& b) {
	a.swap(b);
}


// ----------------------------------------------------------------------------- : DelayedIndexMaps

// The data for a specific name.
/* Invariant: read_data is initialized <=> unread_data.empty()
 */
template <typename Key, typename Value>
struct DelayedIndexMapsData : public IntrusivePtrBase<DelayedIndexMapsData<Key, Value> > {
	String              unread_data;
	IndexMap<Key,Value> read_data;
};

/// A map<String,IndexMap> where the reading of the index map depends on the name.
/** The reading is delayed until the data to initialize the map with is known.
 *  The only way to access data is using get()
 */
template <typename Key, typename Value>
class DelayedIndexMaps {
  public:
	/// Get the data for a specific name. Initialize the map with init_with (if it is not alread initialized)
	IndexMap<Key,Value>& get(const String& name, const vector<Key>& init_with);
	/// Clear the delayed index map
	void clear();
  private:
	map<String, intrusive_ptr<DelayedIndexMapsData<Key,Value> > > data;
	friend class Reader;
	friend class Writer;
	friend class GetDefaultMember;
	friend class GetMember;
};


// ----------------------------------------------------------------------------- : EOF
#endif
