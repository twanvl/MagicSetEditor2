//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <util/error.hpp>
#include <boost/pool/singleton_pool.hpp>

// ----------------------------------------------------------------------------- : ScriptValue
// Base cases

ScriptValue::operator String() const { return _("[[") + typeName() + _("]]"); }
ScriptValue::operator int()    const { throw ScriptError(_("Can't convert from ")+typeName()+_(" to integer number")); }
ScriptValue::operator double() const { throw ScriptError(_("Can't convert from ")+typeName()+_(" to real number"   )); }
ScriptValue::operator Color()  const { throw ScriptError(_("Can't convert from ")+typeName()+_(" to color"         )); }
ScriptValueP ScriptValue::eval(Context&) const
                                     { throw ScriptError(_("Can't convert from ")+typeName()+_(" to function"      )); }
ScriptValueP ScriptValue::getMember(const String& name) const
                                     { throw (typeName() + _(" has no member '") + name + _("'")); }
ScriptValueP ScriptValue::next()               { throw InternalError(_("Can't convert from ")+typeName()+_(" to iterator")); }
ScriptValueP ScriptValue::makeIterator() const { throw ScriptError(  _("Can't convert from ")+typeName()+_(" to collection")); }
int          ScriptValue::itemCount()    const { throw ScriptError(  _("Can't convert from ")+typeName()+_(" to collection")); }

void ScriptValue::signalDependent(Context&, const Dependency&, const String& name) {}
ScriptValueP ScriptValue::dependencies(   Context&, const Dependency&) const { return script_nil; }


// ----------------------------------------------------------------------------- : Iterators

ScriptType ScriptIterator::type() const { return SCRIPT_OBJECT; }
String ScriptIterator::typeName() const { return _("iterator"); }

// Iterator over a range of integers
class ScriptRangeIterator : public ScriptIterator {
  public:
	// Construct a range iterator with the given bounds (inclusive)
	ScriptRangeIterator(int start, int end)
		: pos(start), end(end) {}
	virtual ScriptValueP next() {
		if (pos <= end) {
			return toScript(pos++);
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
	virtual String typeName() const { return _("integer number"); }
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

ScriptValueP toScript(int v) {
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
	virtual String typeName() const { return _("boolean"); }
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
	virtual String typeName() const { return _("real number"); }
	virtual operator String() const { return String() << value; }
	virtual operator double() const { return value; }
	virtual operator int()    const { return (int)value; }
  private:
	double value;
};

ScriptValueP toScript(double v) {
	return new_intrusive1<ScriptDouble>(v);
}

// ----------------------------------------------------------------------------- : String type

// String values
class ScriptString : public ScriptValue {
  public:
	ScriptString(const String& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_STRING; }
	virtual String typeName() const { return _("string"); }
	virtual operator String() const { return value; }
	virtual operator double() const {
		double d;
		if (value.ToDouble(&d)) {
			return d;
		} else {
			throw ScriptError(_("Not a number: '") + value + _("'"));
		}
	}
	virtual operator int()    const {
		long l;
		if (value.ToLong(&l)) {
			return l;
		} else {
			throw ScriptError(_("Not a number: '") + value + _("'"));
		}
	}
	virtual int itemCount() const { return (int)value.size(); }
  private:
	String value;
};

ScriptValueP toScript(const String& v) {
	return new_intrusive1<ScriptString>(v);
}


// ----------------------------------------------------------------------------- : Color

// Color values
class ScriptColor : public ScriptValue {
  public:
	ScriptColor(const Color& v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_COLOR; }
	virtual String typeName() const { return _("color"); }
  private:
	Color value;
};

ScriptValueP toScript(const Color& v) {
	return new_intrusive1<ScriptColor>(v);
}


// ----------------------------------------------------------------------------- : Nil type

// the nil object
class ScriptNil : public ScriptValue {
  public:
	virtual ScriptType type() const { return SCRIPT_NIL; }
	virtual String typeName() const { return _("nil"); }
	virtual operator String() const { return wxEmptyString; }
	virtual operator double() const { return 0.0; }
	virtual operator int()    const { return 0; }
	virtual ScriptValueP eval(Context&) const { return script_nil; } // nil() == nil
};

/// The preallocated nil value
ScriptValueP script_nil(new ScriptNil);


// ----------------------------------------------------------------------------- : EOF
