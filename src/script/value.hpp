//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_VALUE
#define HEADER_SCRIPT_VALUE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/error.hpp>
class Context;
class Dependency;

#ifdef _MSC_VER
	extern "C" {
		LONG  __cdecl _InterlockedIncrement(LONG volatile *Addend);
		LONG  __cdecl _InterlockedDecrement(LONG volatile *Addend);
	}
	#pragma intrinsic (_InterlockedIncrement)
	#define InterlockedIncrement _InterlockedIncrement
	#pragma intrinsic (_InterlockedDecrement)
	#define InterlockedDecrement _InterlockedDecrement
#endif

// ----------------------------------------------------------------------------- : ScriptValue

DECLARE_INTRUSIVE_POINTER_TYPE(ScriptValue);

enum ScriptType
{	SCRIPT_NIL
,	SCRIPT_INT
,	SCRIPT_DOUBLE
,	SCRIPT_STRING
,	SCRIPT_COLOR
,	SCRIPT_IMAGE
,	SCRIPT_FUNCTION
,	SCRIPT_OBJECT // Only ScriptObject
,	SCRIPT_COLLECTION
,	SCRIPT_ITERATOR
,	SCRIPT_DUMMY
};

/// A value that can be handled by the scripting engine.
/// Actual values are derived types
class ScriptValue {
  public:
	inline ScriptValue()
		#ifdef USE_INTRUSIVE_PTR
			 : refCount(0)
		#endif
	{}
	virtual ~ScriptValue() {}
	
	/// Information on the type of this value
	virtual ScriptType type() const = 0;
	/// Name of the type of value
	virtual String typeName() const = 0;
	
	/// Convert this value to a string
	virtual operator String() const;
	/// Convert this value to a double
	virtual operator double() const;
	/// Convert this value to an integer
	virtual operator int()    const;
	/// Convert this value to a color
	virtual operator Color()  const;
		
	/// Get a member variable from this value
	virtual ScriptValueP getMember(const String& name) const;
	/// Signal that a script depends on a member of this value
	/** It is the abstract version of getMember*/
	virtual ScriptValueP dependencyMember(const String& name, const Dependency&) const;
	
	/// Evaluate this value (if it is a function)
	virtual ScriptValueP eval(Context&) const;
	/// Mark the scripts that this function depends on
	/** Return value is an abstract version of the return value of eval */
	virtual ScriptValueP dependencies(Context&, const Dependency&) const;
	
	/// Return an iterator for the current collection, an iterator is a value that has next()
	virtual ScriptValueP makeIterator() const;
	/// Return the next item for this iterator, or ScriptValueP() if there is no such item
	virtual ScriptValueP next();
	/// Return the number of items in this value (assuming it is a collection)
	virtual int itemCount() const;
	
	
  protected:
	/// Delete this object
	virtual void destroy() {
		delete this;
	}
#ifdef USE_INTRUSIVE_PTR
  private:
	volatile LONG refCount;
	friend void intrusive_ptr_add_ref(ScriptValue*);
	friend void intrusive_ptr_release(ScriptValue*);
#endif
};

#ifdef USE_INTRUSIVE_PTR
	inline void intrusive_ptr_add_ref(ScriptValue* p) {
		//p->refCount += 1;
		InterlockedIncrement(&p->refCount);
	}
	inline void intrusive_ptr_release(ScriptValue* p) {
		if (InterlockedDecrement(&p->refCount) == 0) {
		//if (--p->refCount == 0) {
			p->destroy();
		}
	}
#endif

extern ScriptValueP script_nil;   ///< The preallocated nil value
extern ScriptValueP script_true;  ///< The preallocated true value
extern ScriptValueP script_false; ///< The preallocated false value
extern ScriptValueP dependency_dummy; ///< Dummy value used during dependency analysis

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
			return toScript(col->at(pos++));
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
	virtual String typeName() const { return _("collection"); }
	virtual ScriptValueP getMember(const String& name) const {
		long index;
		if (name.ToLong(&index) && index >= 0 && (size_t)index < value->size()) {
			return toScript(value->at(index));
		} else {
			throw ScriptError(_("Collection has no member ") + name);
		}
	}
	virtual ScriptValueP makeIterator() const {
		return new_intrusive1<ScriptCollectionIterator<Collection> >(value);
	}
	virtual int itemCount() const { return (int)value->size(); }
  private:
	/// Store a pointer to a collection, collections are only ever used for structures owned outside the script
	const Collection* value;
};

// ----------------------------------------------------------------------------- : Collections : maps

template <typename V>
ScriptValueP get_member(const map<String,V>& m, const String& name) {
	map<String,V>::const_iterator it = m.find(name);
	if (it != m.end()) {
		return toScript(it->second);
	} else {
		throw ScriptError(_ERROR_1_("collection has no member", name));
	}
}

template <typename K, typename V>
ScriptValueP get_member(const IndexMap<K,V>& m, const String& name) {
	IndexMap<K,V>::const_iterator it = m.find(name);
	if (it != m.end()) {
		return toScript(*it);
	} else {
		throw ScriptError(_ERROR_1_("collection has no member", name));
	}
}

/// Script value containing a map-like collection
template <typename Collection>
class ScriptMap : public ScriptValue {
  public:
	inline ScriptMap(const Collection* v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_COLLECTION; }
	virtual String typeName() const { return _("collection"); }
	virtual ScriptValueP getMember(const String& name) const {
		return get_member(*value, name);
	}
	virtual int itemCount() const { return (int)value->size(); }
  private:
	/// Store a pointer to a collection, collections are only ever used for structures owned outside the script
	const Collection* value;
};

// ----------------------------------------------------------------------------- : Objects

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

/// Script value containing an object (pointer)
template <typename T>
class ScriptObject : public ScriptValue {
  public:
	inline ScriptObject(const T& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_OBJECT; }
	virtual String typeName() const { return _("object"); }
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
				throw  ScriptError(_ERROR_1_("object has no member", name));
			}
		}
	}
	virtual ScriptValueP dependencyMember(const String& name, const Dependency& dep) const {
		mark_dependency_member(value, name, dep);
		return getMember(name);
	}
	virtual ScriptValueP makeIterator() const {
		ScriptValueP it = make_iterator(*value);
		return it ? it : ScriptValue::makeIterator();
	}
	virtual int itemCount() const {
		int i = item_count(*value);
		return i >= 0 ? i : ScriptValue::itemCount();
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

// ----------------------------------------------------------------------------- : Creating

/// Convert a value to a script value
ScriptValueP toScript(int           v);
ScriptValueP toScript(double        v);
ScriptValueP toScript(const String& v);
ScriptValueP toScript(const Color&  v);
inline ScriptValueP toScript(bool                 v) { return v ? script_true : script_false; }
template <typename T>
inline ScriptValueP toScript(const vector<T>*     v) { return new_intrusive1<ScriptCollection<vector<T> > >(v); }
template <typename K, typename V>
inline ScriptValueP toScript(const map<K,V>*      v) { return new_intrusive1<ScriptMap<map<K,V> > >(v); }
template <typename K, typename V>
inline ScriptValueP toScript(const IndexMap<K,V>* v) { return new_intrusive1<ScriptMap<IndexMap<K,V> > >(v); }
template <typename T>
inline ScriptValueP toScript(const shared_ptr<T>& v) { return new_intrusive1<ScriptObject<shared_ptr<T> > >(v); }


// ----------------------------------------------------------------------------- : Buildin functions

/// Macro to declare a new script function
/** Usage:
 *  @code
 *   SCRIPT_FUNCTION(my_function) {
 *      // function code goes here
 *   }
 *  @endcode
 *  This declares a value 'script_my_function' which can be added as a variable to a context
 *  using:
 *  @code
 *   extern ScriptValueP script_my_function;
 *   context.setVariable("my_function", script_my_function);
 *  @endcode
 */
#define SCRIPT_FUNCTION(name) SCRIPT_FUNCTION_AUX(name,virtual)

/// Macro to declare a new script function with custom dependency handling
#define SCRIPT_FUNCTION_DEP(name) SCRIPT_FUNCTION_AUX(name,virtual) //TODO

// helper for SCRIPT_FUNCTION and SCRIPT_FUNCTION_DEP
#define SCRIPT_FUNCTION_AUX(name,dep)							\
		class ScriptBuildin_##name : public ScriptValue {		\
			dep													\
			/* virtual */ ScriptType type() const				\
				{ return SCRIPT_FUNCTION; }						\
			virtual String typeName() const						\
				{ return _("build in function '") _(#name) _("'"); }	\
			virtual ScriptValueP eval(Context&) const;			\
		};														\
		ScriptValueP script_##name(new ScriptBuildin_##name);	\
		ScriptValueP ScriptBuildin_##name::eval(Context& ctx) const

/// Retrieve a parameter to a SCRIPT_FUNCTION with the given name and type
/** Usage:
 *  @code
 *   SCRIPT_FUNCTION(my_function) {
 *      SCRIPT_PARAM(String, my_string_param);
 *      ... my_string_param ...
 *   }
 *  @endcode
 *  Throws an error if the parameter is not found.
 */
#define SCRIPT_PARAM(Type, name)								\
		Type name = getParam<Type>(ctx.getVariable(_(#name)))

template <typename T>
            inline T            getParam              (const ScriptValueP& value) {
				ScriptObject<T>* o = dynamic_cast<ScriptObject<T>*>(value.get());
				if (!o) throw ScriptError(_("Can't convert from ")+value->typeName()+_(" to object"));
				return o->getValue();
            }
template <> inline ScriptValueP getParam<ScriptValueP>(const ScriptValueP& value) { return value;  }
template <> inline String       getParam<String>      (const ScriptValueP& value) { return *value; }
template <> inline int          getParam<int>         (const ScriptValueP& value) { return *value; }
template <> inline double       getParam<double>      (const ScriptValueP& value) { return *value; }

/// Retrieve an optional parameter
/** Usage:
 *  @code
 *   SCRIPT_FUNCTION(my_function) {
 *      SCRIPT_OPTIONAL_PARAM(String, my_string_param) {
 *          ... my_string_param ...
 *      }
 *      ...
 *   }
 *  @endcode
 */
#define SCRIPT_OPTIONAL_PARAM(Type, name)	SCRIPT_OPTIONAL_PARAM_N(Type, #name, name)

#define SCRIPT_OPTIONAL_PARAM_N(Type, str, name)					\
		ScriptValueP name##_ = ctx.getVariableOpt(_(str));			\
		Type name = name##_ ? getParam<Type>(name##_) : Type();		\
		if (name##_)

/// Retrieve an optional parameter, can't be used as an if statement
#define SCRIPT_OPTIONAL_PARAM_(Type, name)	SCRIPT_OPTIONAL_PARAM_N_(Type, #name, name)

#define SCRIPT_OPTIONAL_PARAM_N_(Type, str, name)					\
		ScriptValueP name##_ = ctx.getVariableOpt(_(str));			\
		Type name = name##_ ? getParam<Type>(name##_) : Type();

/// Retrieve an optional parameter with a default value
#define SCRIPT_PARAM_DEFAULT(Type, name, def)						\
		ScriptValueP name##_ = ctx.getVariableOpt(_(#name));		\
		Type name = name##_ ? getParam<Type>(name##_) : def

/// Return a value from a SCRIPT_FUNCTION
#define SCRIPT_RETURN(value) return toScript(value)

// ----------------------------------------------------------------------------- : EOF
#endif
