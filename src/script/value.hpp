//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_VALUE
#define HEADER_SCRIPT_VALUE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/error.hpp>
#include <boost/intrusive_ptr.hpp>
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

//DECLARE_POINTER_TYPE(ScriptValue);
class ScriptValue;
typedef boost::intrusive_ptr<ScriptValue> ScriptValueP;

enum ScriptType
{	SCRIPT_NIL
,	SCRIPT_INT
,	SCRIPT_DOUBLE
,	SCRIPT_STRING
,	SCRIPT_COLOR
,	SCRIPT_BUILDIN_FUN
,	SCRIPT_SCRIPT_FUN
,	SCRIPT_OBJECT
,	SCRIPT_DUMMY
};

/// A value that can be handled by the scripting engine.
/// Actual values are derived types
class ScriptValue {
  public:
	inline ScriptValue() : refCount(0) {}
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
	
	/// Evaluate this value (if it is a function)
	virtual ScriptValueP eval(Context&) const;
	
	/// Return an iterator for the current collection, an iterator is a value that has next()
	virtual ScriptValueP makeIterator() const;
	/// Return the next item for this iterator, or ScriptValueP() if there is no such item
	virtual ScriptValueP next();
	
	/// Signal that a script depends on a member of this value
	virtual void signalDependent(Context&, const Dependency&, const String& name);
	/// Mark the scripts that this function depends on
	/** Return value is an abstract version of the return value of eval */
	virtual ScriptValueP dependencies(Context&, const Dependency&) const;
	
  protected:
	/// Delete this object
	virtual void destroy() {
		delete this;
	}
  private:
	volatile LONG refCount;
	friend void intrusive_ptr_add_ref(ScriptValue*);
	friend void intrusive_ptr_release(ScriptValue*);
};

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

extern ScriptValueP scriptNil;   ///< The preallocated nil value
extern ScriptValueP scriptTrue;  ///< The preallocated true value
extern ScriptValueP scriptFalse; ///< The preallocated false value


// ----------------------------------------------------------------------------- : Iterators

// Iterator over a collection
struct ScriptIterator : public ScriptValue {
	virtual ScriptType type() const;// { return SCRIPT_OBJECT; }
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
	virtual ScriptType type() const { return SCRIPT_OBJECT; }
	virtual String typeName() const { return _("collection"); }
	virtual ScriptValueP getMember(const String& name) const {
		long index;
		if (name.ToLong(&index)) {
			return toScript(value->at(index));
		} else {
			throw ScriptError(_("Collection has no member ") + name);
		}
	}
	virtual ScriptValueP makeIterator() const {
		return new_intrusive1<ScriptCollectionIterator<Collection> >(value);
	}
  private:
	/// Store a pointer to a collection, collections are only ever used for structures owned outside the script
	const Collection* value;
};

// ----------------------------------------------------------------------------- : Objects

/// Script value containing an object (pointer)
template <typename T>
class ScriptObject : public ScriptValue {
  public:
	inline ScriptObject(const shared_ptr<T>& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_OBJECT; }
	virtual String typeName() const { return _("object"); }
	virtual ScriptValueP getMember(const String& name) const {
		GetMember gm(name);
		gm.handle(*value);
		if (gm.result()) return gm.result();
		else             throw  ScriptError(_("Object has no member '") + name + _("'"));
	}
  private:
	shared_ptr<T> value; ///< The object
};

// ----------------------------------------------------------------------------- : Creating

/// Convert a value to a script value
ScriptValueP toScript(int           v);
ScriptValueP toScript(double        v);
ScriptValueP toScript(const String& v);
ScriptValueP toScript(const Color&  v);
inline ScriptValueP toScript(bool                 v) { return v ? scriptTrue : scriptFalse; }
template <typename T>
inline ScriptValueP toScript(const vector<T>*     v) { return new_intrusive1<ScriptCollection<vector<T> > >(v); }
template <typename T>
inline ScriptValueP toScript(const shared_ptr<T>& v) { return new_intrusive1<ScriptObject<T> >(v); }


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
				{ return SCRIPT_BUILDIN_FUN; }					\
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
 *      ...
 *   }
 *  @endcode
 *  Throws an error if the parameter is not found.
 */
#define SCRIPT_PARAM(Type, name)								\
		Type name = *ctx.getVariable(_(#name))

/// Return a value from a SCRIPT_FUNCTION
#define SCRIPT_RETURN(value) return toScript(value)

// ----------------------------------------------------------------------------- : EOF
#endif
