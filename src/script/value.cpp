//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/value.hpp>
#include <util/error.hpp>
#include <boost/lexical_cast.hpp> //%%
#include <boost/pool/singleton_pool.hpp>

// ----------------------------------------------------------------------------- : ScriptValue
// Base cases

ScriptValue::operator String() const { return "[[" + typeName() + "]]"; }
ScriptValue::operator int()    const { throw ScriptError("Can't convert from "+typeName()+" to integer number"); }
ScriptValue::operator double() const { throw ScriptError("Can't convert from "+typeName()+" to real number"   ); }
ScriptValue::operator Color()  const { throw ScriptError("Can't convert from "+typeName()+" to color"         ); }
ScriptValueP ScriptValue::eval(Context&) const
                                     { throw ScriptError("Can't convert from "+typeName()+" to function"      ); }
ScriptValueP ScriptValue::getMember(const String& name) const
                                     { throw (typeName() + " has no member '" + name + "'"); }
ScriptValueP ScriptValue::next()               { throw InternalError("Can't convert from "+typeName()+" to iterator"); }
ScriptValueP ScriptValue::makeIterator() const { throw ScriptError("Can't convert from "+typeName()+" to collection"); }

void ScriptValue::signalDependent(Context&, const Dependency&, const String& name) {}
ScriptValueP ScriptValue::dependencies(   Context&, const Dependency&) const { return scriptNil; }


// ----------------------------------------------------------------------------- : Iterators

ScriptType ScriptIterator::type() const { return SCRIPT_OBJECT; }
String ScriptIterator::typeName() const { return "iterator"; }

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

// Integer values
class ScriptInt : public ScriptValue {
  public:
	ScriptInt(int v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_INT; }
	virtual String typeName() const { return "integer number"; }
	virtual operator String() const { return lexical_cast<String>(value); }
	virtual operator double() const { return value; }
	virtual operator int()    const { return value; }
  protected:
	virtual void destroy() {
		boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::free(this);
	}
  private:
	int value;
};

ScriptValueP toScript(int v) {
//	return new_intrusive1<ScriptInt>(v);
	return ScriptValueP(
			new(boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::malloc())
				ScriptInt(v));
}

// use integers to represent true/false
ScriptValueP scriptTrue  = toScript((int)true);
ScriptValueP scriptFalse = toScript((int)false);


// ----------------------------------------------------------------------------- : Doubles

// Double values
class ScriptDouble : public ScriptValue {
  public:
	ScriptDouble(double v) : value(v) {}
	virtual ScriptType type() const { return SCRIPT_DOUBLE; }
	virtual String typeName() const { return "real number"; }
	virtual operator String() const { return lexical_cast<String>(value); }
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
	virtual String typeName() const { return "string"; }
	virtual operator String() const { return value; }
	virtual operator double() const { return lexical_cast<double>(value); }
	virtual operator int()    const { return lexical_cast<int>(value); }
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
	virtual String typeName() const { return "color"; }
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
	virtual String typeName() const { return "nil"; }
	virtual operator String() const { return ""; }
	virtual operator double() const { return 0.0; }
	virtual operator int()    const { return 0; }
};

/// The preallocated nil value
ScriptValueP scriptNil(new ScriptNil);


// ----------------------------------------------------------------------------- : EOF
