//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_TO_VALUE
#define HEADER_SCRIPT_TO_VALUE

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <script/script.hpp>
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

/// Type name of an object, for error messages
template <typename T> inline String type_name(const T&) {
	return _TYPE_("object");
}
template <typename T> inline String type_name(const intrusive_ptr<T>& p) {
	return type_name(*p.get());
}
template <typename K, typename V> inline String type_name(const pair<K,V>& p) {
	return type_name(p.second); // for maps
}

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
	virtual operator AColor() const;
	virtual int itemCount() const;
	virtual CompareWhat compareAs(String&, void const*&) const;
	// these can propagate the error
	virtual ScriptValueP getMember(const String& name) const;
	virtual ScriptValueP dependencyMember(const String& name, const Dependency&) const;
	virtual ScriptValueP eval(Context&) const;
	virtual ScriptValueP dependencies(Context&, const Dependency&) const;
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const;
  private:
	ScriptError error; // the error message
};

inline ScriptValueP delayError(const String& m) {
	return new_intrusive1<ScriptDelayedError>(ScriptError(m));
}

// ----------------------------------------------------------------------------- : Iterators

// Iterator over a collection
struct ScriptIterator : public ScriptValue {
	virtual ScriptType type() const;// { return SCRIPT_ITERATOR; }
	virtual String typeName() const;// { return "iterator"; }
	virtual CompareWhat compareAs(String&, void const*&) const; // { return COMPARE_NO; }
	
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
	virtual String typeName() const { return _TYPE_1_("collection of", type_name(*value->begin())); }
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
	virtual CompareWhat compareAs(String&, void const*& compare_ptr) const {
		compare_ptr = value;
		return COMPARE_AS_POINTER;
	}
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
		return delayError(_ERROR_2_("has no member", _TYPE_("collection"), name));
	}
}

template <typename K, typename V>
ScriptValueP get_member(const IndexMap<K,V>& m, const String& name) {
	typename IndexMap<K,V>::const_iterator it = m.find(name);
	if (it != m.end()) {
		return to_script(*it);
	} else {
		return delayError(_ERROR_2_("has no member", _TYPE_("collection"), name));
	}
}

/// Script value containing a map-like collection
template <typename Collection>
class ScriptMap : public ScriptValue {
  public:
	inline ScriptMap(const Collection* v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_COLLECTION; }
	virtual String typeName() const { return _TYPE_1_("collection of", type_name(value->begin())); }
	virtual ScriptValueP getMember(const String& name) const {
		return get_member(*value, name);
	}
	virtual int itemCount() const { return (int)value->size(); }
	virtual ScriptValueP dependencyMember(const String& name, const Dependency& dep) const {
		mark_dependency_member(*value, name, dep);
		return getMember(name);
	}
	/// Collections can be compared by comparing pointers
	virtual CompareWhat compareAs(String&, void const*& compare_ptr) const {
		compare_ptr = value;
		return COMPARE_AS_POINTER;
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
	virtual CompareWhat compareAs(String&, void const*& compare_ptr) const {
		compare_ptr = this;
		return COMPARE_AS_POINTER;
	}
	
	/// The collection as a list (contains all values)
	vector<ScriptValueP> value;
	/// The collection as a map (contains only the values that have a key)
	map<String,ScriptValueP> key_value;
};

// ----------------------------------------------------------------------------- : Collections : concatenation

/// Script value containing the concatenation of two collections
class ScriptConcatCollection : public ScriptValue {
  public:
	inline ScriptConcatCollection(ScriptValueP a, ScriptValueP b) : a(a), b(b) {}
	virtual ScriptType type() const { return SCRIPT_COLLECTION; }
	virtual String typeName() const { return _TYPE_("collection"); }
	virtual ScriptValueP getMember(const String& name) const;
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const;
	virtual int itemCount() const { return a->itemCount() + b->itemCount(); }
	/// Collections can be compared by comparing pointers
	virtual CompareWhat compareAs(String&, void const*& compare_ptr) const {
		compare_ptr = this;
		return COMPARE_AS_POINTER;
	}
	
  private:
	ScriptValueP a,b;
	friend class ScriptConcatCollectionIterator;
};

// ----------------------------------------------------------------------------- : Objects

/// Script value containing an object (pointer)
template <typename T>
class ScriptObject : public ScriptValue {
  public:
	inline ScriptObject(const T& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_OBJECT; }
	virtual String typeName() const { return type_name(*value); }
	virtual operator String() const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator String(); }
	virtual operator double() const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator double(); }
	virtual operator int()    const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator int(); }
	virtual operator AColor() const { ScriptValueP d = getDefault(); return d ? *d : ScriptValue::operator AColor(); }
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
	virtual CompareWhat compareAs(String& compare_str, void const*& compare_ptr) const {
		ScriptValueP d = getDefault();
		if (d) {
			return d->compareAs(compare_str, compare_ptr);
		} else {
			compare_ptr = &*value;
			return COMPARE_AS_POINTER;
		}
	}
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

// ----------------------------------------------------------------------------- : Default arguments / closure

/// A wrapper around a function that gives default arguments
class ScriptClosure : public ScriptValue {
  public:
	ScriptClosure(ScriptValueP fun) : fun(fun) {}
	
	virtual ScriptType type() const;
	virtual String typeName() const;
	virtual ScriptValueP eval(Context& ctx) const;
	virtual ScriptValueP dependencies(Context& ctx, const Dependency& dep) const;
	
	/// Add a binding
	void addBinding(Variable v, const ScriptValueP& value);
	/// Is there a binding for the given variable? If so, retrieve it
	ScriptValueP getBinding(Variable v) const;
	
	/// Try to simplify this closure, returns a value if successful
	ScriptValueP simplify();
	
	/// The wrapped function
	ScriptValueP                          fun;
	/// The default argument bindings
	vector<pair<Variable,ScriptValueP> >  bindings;
	
  private:
	/// Apply the bindings in a context
	void applyBindings(Context& ctx) const;
};

// ----------------------------------------------------------------------------- : Creating

/// Convert a value to a script value
       ScriptValueP to_script(int           v);
inline ScriptValueP to_script(long          v) { return to_script((int) v); }
       ScriptValueP to_script(double        v);
       ScriptValueP to_script(const String& v);
       ScriptValueP to_script(Color         v);
       ScriptValueP to_script(AColor        v);
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
template <> inline Color        from_script<Color>       (const ScriptValueP& value) { return (AColor)*value; }
template <> inline AColor       from_script<AColor>      (const ScriptValueP& value) { return *value; }

void from_script(const ScriptValueP& value, wxRegEx& out);

// ----------------------------------------------------------------------------- : EOF
#endif
