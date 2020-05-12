//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/tagged_string.hpp>
#include <script/value.hpp>
#include <script/to_value.hpp>
#include <script/context.hpp>
#include <gfx/generated_image.hpp>
#include <util/error.hpp>
#include <boost/pool/singleton_pool.hpp>

// ----------------------------------------------------------------------------- : ScriptValue
// Base cases

String ScriptValue::toString() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("string"));
}
int ScriptValue::toInt() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("integer"));
}
bool ScriptValue::toBool() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("boolean"));
}
double ScriptValue::toDouble() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("double"));
}
Color ScriptValue::toColor() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("color"));
}
wxDateTime ScriptValue::toDateTime() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("date"));
}
GeneratedImageP ScriptValue::toImage() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("image"   ));
}
String ScriptValue::toCode() const {
  return toString();
}

ScriptValueP ScriptValue::eval(Context&, bool) const {
  return delay_error(ScriptErrorConversion(typeName(), _TYPE_("function")));
}

ScriptValueP ScriptValue::next(ScriptValueP* key_out, int* index_out) {
  throw InternalError(_("Can't convert from ")+typeName()+_(" to iterator"));
}
ScriptValueP ScriptValue::makeIterator() const {
  return delay_error(ScriptErrorConversion(typeName(), _TYPE_("collection")));
}
int ScriptValue::itemCount() const {
  throw ScriptErrorConversion(typeName(), _TYPE_("collection"));
}

CompareWhat  ScriptValue::compareAs(String& compare_str, void const*& compare_ptr) const {
  compare_str = toCode();
  return COMPARE_AS_STRING;
}
ScriptValueP ScriptValue::getMember(const String& name) const {
  long index;
  if (name.ToLong(&index)) {
    return getIndex(index);
  } else {
    return delay_error(ScriptErrorNoMember(typeName(), name));
  }
}
ScriptValueP ScriptValue::getIndex(int index) const {
  return delay_error(ScriptErrorNoMember(typeName(), String()<<index));
}


ScriptValueP ScriptValue::simplifyClosure(ScriptClosure&) const {
  return nullptr;
}

ScriptValueP ScriptValue::dependencyMember(const String& name, const Dependency&) const {
  return dependency_dummy;
}
ScriptValueP ScriptValue::dependencyName(const ScriptValue& container, const Dependency& dep) const {
  return container.dependencyMember(toString(),dep);
}
ScriptValueP ScriptValue::dependencies(Context&, const Dependency&) const {
  return dependency_dummy;
}
void ScriptValue::dependencyThis(const Dependency& dep) {}

bool approx_equal(double a, double b) {
  return a == b || fabs(a - b) < 1e-14;
}

/// compare script values for equallity
bool equal(const ScriptValueP& a, const ScriptValueP& b) {
  if (a == b) return true;
  ScriptType at = a->type(), bt = b->type();
  if (at == bt && at == SCRIPT_INT) {
    return a->toInt() == b->toInt();
  } else if (at == bt && at == SCRIPT_BOOL) {
    return a->toBool() == b->toBool();
  } else if ((at == SCRIPT_INT || at == SCRIPT_DOUBLE) &&
             (bt == SCRIPT_INT || bt == SCRIPT_DOUBLE)) {
    return approx_equal(a->toDouble(), b->toDouble());
  } else if (at == SCRIPT_COLLECTION && bt == SCRIPT_COLLECTION) {
    // compare each element
    if (a->itemCount() != b->itemCount()) return false;
    ScriptValueP a_it = a->makeIterator();
    ScriptValueP b_it = b->makeIterator();
    while (true) {
      ScriptValueP a_v = a_it->next();
      ScriptValueP b_v = b_it->next();
      if (!a_v || !b_v) return a_v == b_v;
      if (!equal(a_v, b_v)) return false;
    }
  } else {
    String      as,  bs;
    const void* ap, *bp;
    CompareWhat aw = a->compareAs(as, ap);
    CompareWhat bw = b->compareAs(bs, bp);
    // compare pointers or strings
    if (aw == COMPARE_AS_STRING || bw == COMPARE_AS_STRING) {
      return as == bs;
    } else if (aw == COMPARE_AS_POINTER || bw == COMPARE_AS_POINTER) {
      return ap == bp;
    } else {
      return false;
    }
  }
}

// ----------------------------------------------------------------------------- : Errors

/// A delayed error message.
/** Only when trying to use the object will the error be thrown.
 *  This can be 'caught' by the "or else" construct
 */
class ScriptDelayedError : public ScriptValue {
public:
  inline ScriptDelayedError(const ScriptError& error) : error(error) {}

  ScriptType type() const override { return SCRIPT_ERROR; }

  // all of these throw
  String typeName() const override { throw error; }
  String toString() const override { throw error; }
  double toDouble() const override { throw error; }
  int toInt() const override { throw error; }
  bool toBool() const override { throw error; }
  Color toColor() const override { throw error; }
  wxDateTime toDateTime() const override { throw error; }
  GeneratedImageP toImage() const override { throw error; }
  int itemCount() const override { throw error; }
  CompareWhat compareAs(String&, void const*&) const override { throw error; }

  // these can propagate the error
  ScriptValueP getMember(const String& name) const override { return delay_error(error); }
  ScriptValueP dependencyMember(const String& name, const Dependency&) const override { return delay_error(error); }
  ScriptValueP dependencies(Context&, const Dependency&) const override { return delay_error(error); }
  ScriptValueP makeIterator() const override { return delay_error(error); }
  ScriptValueP eval(Context&, bool openScope) const override { return delay_error(error); }

private:
  ScriptError error; // the error message
};

ScriptValueP delay_error(const ScriptError& error) {
  return make_intrusive<ScriptDelayedError>(error);
}

// ----------------------------------------------------------------------------- : Iterators

ScriptType ScriptIterator::type() const {
  return SCRIPT_ITERATOR;
}
String ScriptIterator::typeName() const {
  return _("iterator");
}
CompareWhat ScriptIterator::compareAs(String&, void const*&) const {
  return COMPARE_NO;
}
ScriptValueP ScriptIterator::makeIterator() const {
  return const_cast<ScriptIterator*>(this)->intrusive_from_this();
}

// Iterator over a range of integers
class ScriptRangeIterator : public ScriptIterator {
public:
  // Construct a range iterator with the given bounds (inclusive)
  ScriptRangeIterator(int start, int end)
    : pos(start), start(start), end(end)
  {}
  ScriptValueP next(ScriptValueP* key_out, int* index_out) override {
    if (pos <= end) {
      if (key_out) *key_out = to_script(pos-start);
      if (index_out) *index_out = (int)pos;
      return to_script(pos++);
    } else {
      return ScriptValueP();
    }
  }
private:
  int pos, start, end;
};

ScriptValueP rangeIterator(int start, int end) {
  return make_intrusive<ScriptRangeIterator>(start, end);
}

// ----------------------------------------------------------------------------- : Integers

#if defined(USE_INTRUSIVE_PTR) && !defined(USE_POOL_ALLOCATOR)
  #define USE_POOL_ALLOCATOR 0 // disabled by default
#endif

// Integer values
class ScriptInt : public ScriptValue {
public:
  ScriptInt(int v) : value(v) {}
  ScriptType type() const override { return SCRIPT_INT; }
  String typeName() const override { return _TYPE_("integer"); }
  String toString() const override { return String() << value; }
  double toDouble() const override { return value; }
  int toInt()       const override { return value; }
protected:
#if USE_POOL_ALLOCATOR
  void destroy() const override {
    boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::free(this);
  }
#endif
private:
  int value;
};

#if USE_POOL_ALLOCATOR && !USE_INTRUSIVE_PTR
  // deallocation function for pool allocated integers
  void destroy_value(ScriptInt* v) {
    boost::singleton_pool<ScriptValue, sizeof(ScriptInt)>::free(v);
  }
#endif

ScriptValueP to_script(int v) {
#if USE_POOL_ALLOCATOR
  #if USE_INTRUSIVE_PTR
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
  return make_intrusive<ScriptInt>(v);
#endif
}

// ----------------------------------------------------------------------------- : Booleans

// Boolean values
class ScriptBool : public ScriptValue {
public:
  ScriptBool(bool v) : value(v) {}
  ScriptType type() const override { return SCRIPT_BOOL; }
  String typeName() const override { return _TYPE_("boolean"); }
  String toString() const override { return value ? _("true") : _("false"); }
  bool toBool() const override { return value; }
  // bools don't autoconvert to int
private:
  bool value;
};

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
  ScriptType type() const override { return SCRIPT_DOUBLE; }
  String typeName() const override { return _TYPE_("double"); }
  String toString() const override { return String() << value; }
  double toDouble() const override { return value; }
  int toInt() const override { return (int)value; }
private:
  double value;
};

ScriptValueP to_script(double v) {
  return make_intrusive<ScriptDouble>(v);
}

// ----------------------------------------------------------------------------- : String type

String quote_string(String const& str) {
  String out;
  out.reserve(str.size() + 2);
  out += _('"');
  FOR_EACH_CONST(c, str) {
    if      (c == _('"') || c == _('\\')) { out += _('\\'); out += c; }
    else if (c == ESCAPED_LANGLE) out += _("\\<");
    else if (c == _('\n')) out += _("\\n");
    else if (c == _('\r')) out += _("\\r");
    else if (c == _('\t')) out += _("\\t");
    else out += c;
  }
  out += _('"');
  return out;
}

// String values
class ScriptString : public ScriptValue {
public:
  ScriptString(const String& v) : value(v) {}
  ScriptType type() const override { return SCRIPT_STRING; }
  String typeName() const override { return _TYPE_("string") + _(" (\"") + (value.size() < 30 ? value : value.substr(0,30) + _("...")) + _("\")"); }
  String toString() const override { return value; }
  String toCode() const override {
    return quote_string(value);
  }
  double toDouble() const override {
    double d;
    if (value.ToDouble(&d)) {
      return d;
    } else {
      throw ScriptErrorConversion(value, typeName(), _TYPE_("double"));
    }
  }
  int toInt() const override {
    long l;
    if (value.ToLong(&l)) {
      return l;
    } else {
      throw ScriptErrorConversion(value, typeName(), _TYPE_("integer"));
    }
  }
  bool toBool() const override {
    if (value == _("yes") || value == _("true")) {
      return true;
    } else if (value == _("no") || value == _("false") || value.empty()) {
      return false;
    } else {
      throw ScriptErrorConversion(value, typeName(), _TYPE_("boolean"));
    }
  }
  Color toColor() const override {
    optional<Color> c = parse_color(value);
    if (!c.has_value()) {
      throw ScriptErrorConversion(value, typeName(), _TYPE_("color"));
    }
    return *c;
  }
  wxDateTime toDateTime() const override {
    wxDateTime date;
    String::const_iterator end;
    if (!date.ParseDateTime(value,&end) || end != value.end()) {
      throw ScriptErrorConversion(value, typeName(), _TYPE_("date"));
    }
    return date;
  }
  GeneratedImageP toImage() const override {
    if (value.empty()) {
      return make_intrusive<BlankImage>();
    } else {
      return make_intrusive<PackagedImage>(value);
    }
  }
  int itemCount() const override { return (int)value.size(); }
  ScriptValueP getMember(const String& name) const override {
    // get member returns characters
    long index;
    if (name.ToLong(&index) && index >= 0 && (size_t)index < value.size()) {
      return to_script(String(1,value[index]));
    } else {
      return delay_error(_ERROR_2_("has no member value", value, name));
    }
  }
private:
  String value;
};

ScriptValueP to_script(const String& v) {
  return make_intrusive<ScriptString>(v);
}


// ----------------------------------------------------------------------------- : Color

// Color values
class ScriptColor : public ScriptValue {
public:
  ScriptColor(const Color& v) : value(v) {}
  ScriptType type() const override { return SCRIPT_COLOR; }
  String typeName() const override { return _TYPE_("color"); }
  Color toColor() const override { return value; }
  // colors don't auto convert to int, use to_int to force
  String toString() const override {
    return format_color(value);
  }
private:
  Color value;
};

ScriptValueP to_script(Color v) {
  return make_intrusive<ScriptColor>(v);
}


// ----------------------------------------------------------------------------- : DateTime

// wxDateTime values
class ScriptDateTime : public ScriptValue {
public:
  ScriptDateTime(const wxDateTime& v) : value(v) {}
  ScriptType type() const override { return SCRIPT_DATETIME; }
  String typeName() const override { return _TYPE_("date"); }
  wxDateTime toDateTime() const override { return value; }
  String toString() const override {
    return value.Format(_("%Y-%m-%d %H:%M:%S"));
  }
private:
  wxDateTime value;
};

ScriptValueP to_script(wxDateTime v) {
  return make_intrusive<ScriptDateTime>(v);
}


// ----------------------------------------------------------------------------- : Nil type

// the nil object
class ScriptNil : public ScriptValue {
public:
  ScriptType type() const override { return SCRIPT_NIL; }
  String typeName() const override { return _TYPE_("nil"); }
  String toString() const override { return String(); }
  double toDouble() const override { return 0.0; }
  int toInt() const override { return 0; }
  bool toBool() const override { return false; }
  Color toColor() const override { return Color(); }
  GeneratedImageP toImage() const override {
    return make_intrusive<BlankImage>();
  }
  String toCode() const override {
    return "nil";
  }
  ScriptValueP eval(Context& ctx, bool) const override {
    // nil(input) == input
    return ctx.getVariable(SCRIPT_VAR_input);
  }
};

/// The preallocated nil value
ScriptValueP script_nil = make_intrusive<ScriptNil>();

// ----------------------------------------------------------------------------- : Collection base

String ScriptCollectionBase::toCode() const {
  String ret = _("[");
  bool first = true;
  ScriptValueP it = makeIterator();
  ScriptValueP key;
  int index = -1;
  while (ScriptValueP v = it->next(&key,&index)) {
    if (!first) ret += _(",");
    first = false;
    if (index == -1) {
      ret += key->toCode();
      ret += _(":");
    }
    ret += v->toCode();
  }
  ret += _("]");
  return ret;
}

// ----------------------------------------------------------------------------- : Custom collection

// Iterator over a custom collection
class ScriptCustomCollectionIterator : public ScriptIterator {
public:
  ScriptCustomCollectionIterator(intrusive_ptr<ScriptCustomCollection const> const& col)
    : col(col), pos(0), it(col->key_value.begin()) {}
  ScriptValueP next(ScriptValueP* key_out, int* index_out) override {
    if (pos < col->value.size()) {
      if (key_out) *key_out = to_script((int)pos);
      if (index_out) *index_out = (int)pos;
      return col->value.at(pos++);
    } else if (it != col->key_value.end()) {
      if (key_out) *key_out = to_script(it->first);
      if (index_out) *index_out = -1;
      return (it++)->second;
    } else {
      return ScriptValueP();
    }
  }
private:
  intrusive_ptr<ScriptCustomCollection const> col;
  size_t pos;
  map<String,ScriptValueP>::const_iterator it;
};

ScriptValueP ScriptCustomCollection::getMember(const String& name) const {
  map<String,ScriptValueP>::const_iterator it = key_value.find(name);
  if (it != key_value.end()) {
    return it->second;
  } else {
    return ScriptValue::getMember(name);
  }
}
ScriptValueP ScriptCustomCollection::getIndex(int index) const {
  if (index >= 0 && (size_t)index < value.size()) {
    return value.at(index);
  } else {
    return ScriptValue::getIndex(index);
  }
}
ScriptValueP ScriptCustomCollection::makeIterator() const {
  return make_intrusive<ScriptCustomCollectionIterator>(const_cast<ScriptCustomCollection*>(this)->intrusive_from_this());
}

// ----------------------------------------------------------------------------- : Concat collection

// Iterator over a concatenated collection
class ScriptConcatCollectionIterator : public ScriptIterator {
public:
  ScriptConcatCollectionIterator(const ScriptValueP& itA, const ScriptValueP& itB)
    : itA(itA), itB(itB) {}
  ScriptValueP next(ScriptValueP* key_out, int* index_out) override {
    if (itA) {
      ScriptValueP v = itA->next(key_out, index_out);
      if (v) return v;
      else   itA = ScriptValueP();
    }
    // TODO: somehow fix up the keys
    return itB->next(key_out, index_out);
  }
private:
  ScriptValueP itA, itB;
};

ScriptValueP ScriptConcatCollection::getMember(const String& name) const {
  ScriptValueP member = a->getMember(name);
  if (member->type() != SCRIPT_ERROR) return member;
  long index;
  int  itemsInA = a->itemCount();
  if (name.ToLong(&index) && index - itemsInA >= 0 && index - itemsInA < b->itemCount()) { 
    // adjust integer index
    return b->getMember(String() << (index - itemsInA));
  } else {
    return b->getMember(name);
  }
}
ScriptValueP ScriptConcatCollection::getIndex(int index) const {
  int itemsInA = a->itemCount();
  if (index < itemsInA) {
    return a->getIndex(index);
  } else {
    return b->getIndex(index - itemsInA);
  }
}
ScriptValueP ScriptConcatCollection::makeIterator() const {
  return make_intrusive<ScriptConcatCollectionIterator>(a->makeIterator(), b->makeIterator());
}

// ----------------------------------------------------------------------------- : Default arguments / closure

ScriptType ScriptClosure::type() const {
  return SCRIPT_FUNCTION;
}
String ScriptClosure::typeName() const {
  return fun->typeName() + _(" closure");
}

void ScriptClosure::addBinding(Variable v, const ScriptValueP& value) {
  bindings.push_back(make_pair(v,value));
}
ScriptValueP ScriptClosure::getBinding(Variable v) const {
  FOR_EACH_CONST(b, bindings) {
    if (b.first == v) return b.second;
  }
  return ScriptValueP();
}

ScriptValueP ScriptClosure::simplify() {
  return fun->simplifyClosure(*this);
}

ScriptValueP ScriptClosure::eval(Context& ctx, bool openScope) const {
  unique_ptr<LocalScope> scope = openScope ? make_unique<LocalScope>(ctx) : nullptr;
  applyBindings(ctx);
  return fun->eval(ctx, openScope);
}
ScriptValueP ScriptClosure::dependencies(Context& ctx, const Dependency& dep) const {
  LocalScope scope(ctx);
  applyBindings(ctx);
  return fun->dependencies(ctx, dep);
}
void ScriptClosure::applyBindings(Context& ctx) const {
  FOR_EACH_CONST(b, bindings) {
    if (ctx.getVariableScope(b.first) != 0) {
      // variables passed as arguments (i.e. in scope 0) override these default bindings
      ctx.setVariable(b.first, b.second);
    }
  }
}


ScriptType ScriptRule::type() const { return SCRIPT_FUNCTION; }
String ScriptRule::typeName() const { return fun->typeName() + _("_rule"); }
ScriptValueP ScriptRule::eval(Context& ctx, bool openScope) const {
  return ctx.makeClosure(fun);
}
