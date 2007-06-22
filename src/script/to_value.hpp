//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_TO_VALUE
#define HEADER_SCRIPT_TO_VALUE

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <util/reflect.hpp>
#include <util/error.hpp>
#include <util/io/get_member.hpp>

// ----------------------------------------------------------------------------- : Overloadable templates

/// Number of items in some collection like object, can be overloaded
template <typename T>
int item_count(const T& v) {
	return -1;
}
/// Return an iterator for some collection, can be overloaded
template <typename T>
ScriptValueP make_iterator(const T& v) {
	return ScriptValueP();
}

/// Mark a dependency on a member of value, can be overloaded
template <typename T>
void mark_dependency_member(const T& value, const String& name, const Dependency& dep) {}

// ----------------------------------------------------------------------------- : Errors

/// A delayed error message.
/** Only when trying to use the object will the error be thrown.
 *  This can be 'caught' by the "or else" construct
 */
class ScriptDelayedError : public ScriptValue {
  public:
	inline ScriptDelayedError(const ScriptError& error) : error(error) {}
	
	virtual ScriptType type() const;// { return SCRIPT_ERROR; }
	
	// all of these throw
	virtual String typeName() const;
	virtual operator String() const;
	virtual operator double() const;
	virtual operator int()    const;
	virtual operator Color()  const;
	virtual int itemCount() const;
	virtual const void* comparePointer() const;
	// these can propagate the error
	virtual ScriptValueP getMember(const String& name) const;
	virtual ScriptValueP dependencyMember(const String& name, const Dependency&) const;
	virtual ScriptValueP eval(Context&) const;
	virtual ScriptValueP dependencies(Context&, const Dependency&) const;
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const;
  private:
	ScriptError error; // the error message
};


// ----------------------------------------------------------------------------- : Iterators

// Iterator over a collection
struct ScriptIterator : public ScriptValue {
	virtual ScriptType type() const;// { return SCRIPT_ITERATOR; }
	virtual String typeName() const;// { return "iterator"; }
	
	/// Return the next item for this iterator, or ScriptValueP() if there is no such item
	virtual ScriptValueP next() = 0;
};

// make an iterator over a range
ScriptValueP rangeIterator(int start, int end);

// ----------------------------------------------------------------------------- : Collections

// Iterator over a collection
template <typename Collection>
class ScriptCollectionIterator : public ScriptIterator {
  public:	
	ScriptCollectionIterator(const Collection* col) : pos(0), col(col) {}
	virtual ScriptValueP next() {
		if (pos < col->size()) {
			return to_script(col->at(pos++));
		} else {
			return ScriptValueP();
		}
	}
  private:
	size_t pos;
	const Collection* col;
};

/// Script value containing a collection
template <typename Collection>
class ScriptCollection : public ScriptValue {
  public:
	inline ScriptCollection(const Collection* v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_COLLECTION; }
	virtual String typeName() const { return _TYPE_("collection"); }
	virtual ScriptValueP getMember(const String& name) const {
		long index;
		if (name.ToLong(&index) && index >= 0 && (size_t)index < value->size()) {
			return to_script(value->at(index));
		} else {
			return ScriptValue::getMember(name);
		}
	}
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const {
		return new_intrusive1<ScriptCollectionIterator<Collection> >(value);
	}
	virtual int itemCount() const { return (int)value->size(); }
	/// Collections can be compared by comparing pointers
	virtual const void* comparePointer() const { return value; }
  private:
	/// Store a pointer to a collection, collections are only ever used for structures owned outside the script
	const Collection* value;
};

// ----------------------------------------------------------------------------- : Collections : maps

template <typename V>
ScriptValueP get_member(const map<String,V>& m, const String& name) {
	typename map<String,V>::const_iterator it = m.find(name);
	if (it != m.end()) {
		return to_script(it->second);
	} else {
		throw ScriptError(_ERROR_2_("has no member", _TYPE_("collection"), name));
	}
}

template <typename K, typename V>
ScriptValueP get_member(const IndexMap<K,V>& m, const String& name) {
	typename IndexMap<K,V>::const_iterator it = m.find(name);
	if (it != m.end()) {
		return to_script(*it);
	} else {
		throw ScriptError(_ERROR_2_("has no member", _TYPE_("collection"), name));
	}
}

/// Script value containing a map-like collection
template <typename Collection>
class ScriptMap : public ScriptValue {
  public:
	inline ScriptMap(const Collection* v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_COLLECTION; }
	virtual String typeName() const { return _TYPE_("collection"); }
	virtual ScriptValueP getMember(const String& name) const {
		return get_member(*value, name);
	}
	virtual int itemCount() const { return (int)value->size(); }
	/// Collections can be compared by comparing pointers
	virtual const void* comparePointer() const { return value; }
	virtual ScriptValueP dependencyMember(const String& name, const Dependency& dep) const {
		mark_dependency_member(*value, name, dep);
		return getMember(name);
	}
  private:
	/// Store a pointer to a collection, collections are only ever used for structures owned outside the script
	const Collection* value;
};

// ----------------------------------------------------------------------------- : Collections : from script

/// Script value containing a custom collection, returned from script functions
class ScriptCustomCollection : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_COLLECTION; }
	virtual String typeName() const { return _TYPE_("collection"); }
	virtual ScriptValueP getMember(const String& name) const;
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const;
	virtual int itemCount() const { return (int)value.size(); }
	/// Collections can be compared by comparing pointers
	virtual const void* comparePointer() const { return &value; }
	
	/// The collection as a list (contains all values)
	vector<ScriptValueP> value;
	/// The collection as a map (contains only the values that have a key)
	map<String,ScriptValueP> key_value;
};

// ----------------------------------------------------------------------------- : Objects

/// Script value containing an object (pointer)
template <typename T>
class ScriptObject : public ScriptValue {
  public:
	inline ScriptObject(const T& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_OBJECT; }
	virtual String typeName() const { return _TYPE_("object"); }
	virtual operator String() const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator String(); }
	virtual operator double() const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator double(); }
	virtual operator int()    const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator int(); }
	virtual operator Color()  const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator Color(); }
	virtual ScriptValueP getMember(const String& name) const {
		GetMember gm(name);
		gm.handle(*value);
		if (gm.result()) return gm.result();
		else {
			// try nameless member
			ScriptValueP d = getDefault();
			if (d) {
				return d->getMember(name);
			} else {
				return ScriptValue::getMember(name);
			}
		}
	}
	virtual ScriptValueP dependencyMember(const String& name, const Dependency& dep) const {
		mark_dependency_member(*value, name, dep);
		return getMember(name);
	}
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const {
		ScriptValueP it = make_iterator(*value);
		return it ? it : ScriptValue::makeIterator(thisP);
	}
	virtual int itemCount() const {
		int i = item_count(*value);
		return i >= 0 ? i : ScriptValue::itemCount();
	}
	/// Objects can be compared by comparing pointers
	virtual const void* comparePointer() const { return &*value; }
	/// Get access to the value
	inline T getValue() const { return value; }
  private:
	T value; ///< The object
	ScriptValueP getDefault() const {
		GetDefaultMember gdm;
		gdm.handle(*value);
		return gdm.result();
	}
};

// ----------------------------------------------------------------------------- : Creating

/// Convert a value to a script value
       ScriptValueP to_script(int           v);
inline ScriptValueP to_script(long          v) { return to_script((int) v); }
       ScriptValueP to_script(double        v);
       ScriptValueP to_script(const String& v);
       ScriptValueP to_script(const Color&  v);
inline ScriptValueP to_script(bool          v) { return v ? script_true : script_false; }
template <typename T>
inline ScriptValueP to_script(const vector<T>*     v) { return new_intrusive1<ScriptCollection<vector<T> > >(v); }
template <typename K, typename V>
inline ScriptValueP to_script(const map<K,V>*      v) { return new_intrusive1<ScriptMap<map<K,V> > >(v); }
template <typename K, typename V>
inline ScriptValueP to_script(const IndexMap<K,V>* v) { return new_intrusive1<ScriptMap<IndexMap<K,V> > >(v); }
template <typename T>
inline ScriptValueP to_script(const intrusive_ptr<T>& v) { return new_intrusive1<ScriptObject<intrusive_ptr<T> > >(v); }
template <typename T>
inline ScriptValueP to_script(const Defaultable<T>& v) { return to_script(v()); }

// ----------------------------------------------------------------------------- : Destructing

/// Convert a value from a script value to a normal value
template <typename T> inline T  from_script              (const ScriptValueP& value) {
	ScriptObject<T>* o = dynamic_cast<ScriptObject<T>*>(value.get());
	if (!o) {
		throw ScriptError(_ERROR_2_("can't convert", value->typeName(), _TYPE_("object" )));
	}
	return o->getValue();
}
template <> inline ScriptValueP from_script<ScriptValueP>(const ScriptValueP& value) { return value;  }
template <> inline String       from_script<String>      (const ScriptValueP& value) { return *value; }
template <> inline int          from_script<int>         (const ScriptValueP& value) { return *value; }
template <> inline double       from_script<double>      (const ScriptValueP& value) { return *value; }
template <> inline bool         from_script<bool>        (const ScriptValueP& value) { return *value; }

// ----------------------------------------------------------------------------- : EOF
#endif
