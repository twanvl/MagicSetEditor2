//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_VALUE
#define HEADER_SCRIPT_VALUE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
class Context;
class Dependency;

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
class ScriptValue : public IntrusivePtrBase {
  public:
	virtual ~ScriptValue() {}
	
	/// Information on the type of this value
	virtual ScriptType type() const = 0;
	/// Name of the type of value
	virtual String typeName() const = 0;
	/// A pointer that uniquely identifies the value, used for comparing.
	/** If implementation is not possible, should return nullptr. */
	virtual const void* comparePointer() const;
	
	/// Convert this value to a string
	virtual operator String() const;
	/// Convert this value to a double
	virtual operator double() const;
	/// Convert this value to an integer
	virtual operator int()    const;
	/// Convert this value to a boolean
	inline  operator bool()   const { return (int)*this; }
	/// Convert this value to a color
	virtual operator Color()  const;
	
	/// Explicit overload to convert to a string
	/** This is sometimes necessary, because wxString has an int constructor,
	 *  which confuses gcc. */
	inline String toString() const { return *this; }
	
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
	/** thisP can be used to prevent destruction of the collection */
	virtual ScriptValueP makeIterator(const ScriptValueP& thisP) const;
	/// Return the next item for this iterator, or ScriptValueP() if there is no such item
	virtual ScriptValueP next();
	/// Return the number of items in this value (assuming it is a collection)
	virtual int itemCount() const;
};

extern ScriptValueP script_nil;   ///< The preallocated nil value
extern ScriptValueP script_true;  ///< The preallocated true value
extern ScriptValueP script_false; ///< The preallocated false value
extern ScriptValueP dependency_dummy; ///< Dummy value used during dependency analysis

// ----------------------------------------------------------------------------- : EOF
#endif
