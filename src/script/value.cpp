//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/value.hpp>
#include <script/to_value.hpp>
#include <util/error.hpp>
#include <boost/pool/singleton_pool.hpp>

// ----------------------------------------------------------------------------- : ScriptValue
// Base cases

ScriptValue::operator String()                              const { throw ScriptError(_ERROR_2_("can't convert", typeName(), _TYPE_("string" ))); }
ScriptValue::operator int()                                 const { throw ScriptError(_ERROR_2_("can't convert", typeName(), _TYPE_("integer" ))); }
ScriptValue::operator double()                              const { throw ScriptError(_ERROR_2_("can't convert", typeName(), _TYPE_("double"  ))); }
ScriptValue::operator Color()                               const { throw ScriptError(_ERROR_2_("can't convert", typeName(), _TYPE_("color"   ))); }
ScriptValueP ScriptValue::eval(Context&)                    const { return delayError(_ERROR_2_("can't convert", typeName(), _TYPE_("function"))); }
ScriptValueP ScriptValue::getMember(const String& name)     const { return delayError(_ERROR_2_("has no member", typeName(), name));              }
ScriptValueP ScriptValue::next()                                  { throw InternalError(_("Can't convert from ")+typeName()+_(" to iterator")); }
ScriptValueP ScriptValue::makeIterator(const ScriptValueP&) const { return delayError(_ERROR_2_("can't convert", typeName(), _TYPE_("collection"))); }
int          ScriptValue::itemCount()                       const { throw ScriptError(_ERROR_2_("can't convert", typeName(), _TYPE_("collection"))); }
const void*  ScriptValue::comparePointer()                  const { return nullptr; }

ScriptValueP ScriptValue::dependencyMember(const String& name, const Dependency&) const { return dependency_dummy; }
ScriptValueP ScriptValue::dependencies(Context&,               const Dependency&) const { return dependency_dummy; }


// ----------------------------------------------------------------------------- : Errors

ScriptType ScriptDelayedError::type() const { return SCRIPT_ERROR; }

String ScriptDelayedError::typeName() const            { throw error; }
ScriptDelayedError::operator String() const            { throw error; }
ScriptDelayedError::operator double() const            { throw error; }
ScriptDelayedError::operator int()    const            { throw error; }
ScriptDelayedError::operator Color()  const            { throw error; }
int ScriptDelayedError::itemCount() const              { throw error; }
const void* ScriptDelayedError::comparePointer() const { throw error; }
ScriptValueP ScriptDelayedError::getMember(const String&) const                           { return new_intrusive1<ScriptDelayedError>(error); }
ScriptValueP ScriptDelayedError::dependencyMember(const String&, const Dependency&) const { return new_intrusive1<ScriptDelayedError>(error); }
ScriptValueP ScriptDelayedError::eval(Context&) const                                     { return new_intrusive1<ScriptDelayedError>(error); }
ScriptValueP ScriptDelayedError::dependencies(Context&, const Dependency&) const          { return new_intrusive1<ScriptDelayedError>(error); }
ScriptValueP ScriptDelayedError::makeIterator(const ScriptValueP& thisP) const            { return thisP; }


// ----------------------------------------------------------------------------- : Iterators

ScriptType ScriptIterator::type() const { return SCRIPT_ITERATOR; }
String ScriptIterator::typeName() const { return _("iterator"); }

// Iterator over a range of integers
class ScriptRangeIterator : public ScriptIterator {
  public:
	// Construct a range iterator with the given bounds (inclusive)
	ScriptRangeIterator(int start, int end)
		: pos(start), end(end) {}
	virtual ScriptValueP next() {
		if (pos <= end) {
			return to_script(pos++);
		} else {
			return ScriptValueP();
		}
	}
  private:
	int pos, end;
};

ScriptValueP rangeIterator(int start, int end) {
	return new_intrusive2<ScriptRangeIterator>(start, end);
}

// ----------------------------------------------------------------------------- : Integers

#define USE_POOL_ALLOCATOR

// Integer values
class ScriptInt : public ScriptValue {
  public:
	ScriptInt(int v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_INT; }
	virtual String typeName() const { return _TYPE_("integer"); }
	virtual operator String() const { return String() << value; }
	virtual operator double() const { return value; }
	virtual operator int()    const { return value; }
  protected:
#ifdef USE_POOL_ALLOCATOR
	virtual void destroy() {
		boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::free(this);
	}
#endif
  private:
	int value;
};

#if defined(USE_POOL_ALLOCATOR) && !defined(USE_INTRUSIVE_PTR)
	// deallocation function for pool allocated integers
	void destroy_value(ScriptInt* v) {
		boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::free(v);
	}
#endif

ScriptValueP to_script(int v) {
#ifdef USE_POOL_ALLOCATOR
	#ifdef USE_INTRUSIVE_PTR
		return ScriptValueP(
				new(boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::malloc())
					ScriptInt(v));
	#else
		return ScriptValueP(
				new(boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::malloc())
					ScriptInt(v),
				destroy_value); // deallocation function
	#endif
#else
	return new_intrusive1<ScriptInt>(v);
#endif
}

// ----------------------------------------------------------------------------- : Booleans

// Boolean values
class ScriptBool : public ScriptValue {
  public:
	ScriptBool(bool v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_INT; }
	virtual String typeName() const { return _TYPE_("boolean"); }
	virtual operator String() const { return value ? _("true") : _("false"); }
	virtual operator int()    const { return value; }
  private:
	bool value;
};

// use integers to represent true/false
/* NOTE: previous versions used ScriptInts as booleans, this gives problems
 * when we use a pool allocator for them, because the pool is destroyed before these globals.
 */
ScriptValueP script_true (new ScriptBool(true));
ScriptValueP script_false(new ScriptBool(false));


// ----------------------------------------------------------------------------- : Doubles

// Double values
class ScriptDouble : public ScriptValue {
  public:
	ScriptDouble(double v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_DOUBLE; }
	virtual String typeName() const { return _TYPE_("double"); }
	virtual operator String() const { return String() << value; }
	virtual operator double() const { return value; }
	virtual operator int()    const { return (int)value; }
  private:
	double value;
};

ScriptValueP to_script(double v) {
	return new_intrusive1<ScriptDouble>(v);
}

// ----------------------------------------------------------------------------- : String type

// String values
class ScriptString : public ScriptValue {
  public:
	ScriptString(const String& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_STRING; }
	virtual String typeName() const { return _TYPE_("string") + _(" (\"") + (value.size() < 30 ? value : value.substr(0,30) + _("...")) + _("\")"); }
	virtual operator String() const { return value; }
	virtual operator double() const {
		double d;
		if (value.ToDouble(&d)) {
			return d;
		} else {
			throw ScriptError(_ERROR_3_("can't convert value", value, typeName(), _TYPE_("double")));
		}
	}
	virtual operator int()    const {
		long l;
		if (value.ToLong(&l)) {
			return l;
		} else if (value == _("yes") || value == _("true")) {
			return true;
		} else if (value == _("no") || value == _("false") || value.empty()) {
			return false;
		} else {
			throw ScriptError(_ERROR_3_("can't convert value", value, typeName(), _TYPE_("integer")));
		}
	}
	virtual operator Color() const {
		UInt r,g,b;
		if (wxSscanf(value.c_str(),_("rgb(%u,%u,%u)"),&r,&g,&b)) {
			return Color(r, g, b);
		} else {
			// color from database?
			Color c(value);
			if (!c.Ok()) {
				throw ScriptError(_ERROR_3_("can't convert value", value, typeName(), _TYPE_("color")));
			}
			return c;
		}
	}
	virtual int itemCount() const { return (int)value.size(); }
	virtual ScriptValueP getMember(const String& name) const {
		// get member returns characters
		long index;
		if (name.ToLong(&index) && index >= 0 && (size_t)index < value.size()) {
			return to_script(String(1,value[index]));
		} else {
			return delayError(_ERROR_2_("has no member value", value, name));
		}
	}
  private:
	String value;
};

ScriptValueP to_script(const String& v) {
	return new_intrusive1<ScriptString>(v);
}


// ----------------------------------------------------------------------------- : Color

// Color values
class ScriptColor : public ScriptValue {
  public:
	ScriptColor(const Color& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_COLOR; }
	virtual String typeName() const { return _TYPE_("color"); }
	virtual operator Color()  const { return value; }
	virtual operator int()    const { return (value.Red() + value.Blue() + value.Green()) / 3; }
	virtual operator String() const {
		return String::Format(_("rgb(%u,%u,%u)"), value.Red(), value.Green(), value.Blue());
	}
  private:
	Color value;
};

ScriptValueP to_script(const Color& v) {
	return new_intrusive1<ScriptColor>(v);
}


// ----------------------------------------------------------------------------- : Nil type

// the nil object
class ScriptNil : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_NIL; }
	virtual String typeName() const { return _TYPE_("nil"); }
	virtual operator String() const { return wxEmptyString; }
	virtual operator double() const { return 0.0; }
	virtual operator int()    const { return 0; }
	virtual ScriptValueP eval(Context&) const { return script_nil; } // nil() == nil
};

/// The preallocated nil value
ScriptValueP script_nil(new ScriptNil);

// ----------------------------------------------------------------------------- : Custom collection

// Iterator over a custom collection
class ScriptCustomCollectionIterator : public ScriptIterator {
  public:	
	ScriptCustomCollectionIterator(const vector<ScriptValueP>* col, ScriptValueP colP)
		: pos(0), col(col), colP(colP) {}
	virtual ScriptValueP next() {
		if (pos < col->size()) {
			return col->at(pos++);
		} else {
			return ScriptValueP();
		}
	}
  private:
	size_t pos;
	const vector<ScriptValueP>* col;
	ScriptValueP colP; // for ownership of the collection
};

ScriptValueP ScriptCustomCollection::getMember(const String& name) const {
	map<String,ScriptValueP>::const_iterator it = key_value.find(name);
	if (it != key_value.end()) {
		return it->second;
	} else {
		long index;
		if (name.ToLong(&index) && index >= 0 && (size_t)index < value.size()) {
			return value.at(index);
		} else {
			return ScriptValue::getMember(name);
		}
	}
}
ScriptValueP ScriptCustomCollection::makeIterator(const ScriptValueP& thisP) const {
	return new_intrusive2<ScriptCustomCollectionIterator>(&value, thisP);
}

