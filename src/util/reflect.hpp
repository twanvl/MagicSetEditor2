//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file util/reflect.hpp
 *
 *  @brief Reflection of classes, currently reflection is used for (de)serialization.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/io/reader.hpp>
#include <util/io/writer.hpp>
#include <util/io/get_member.hpp>

// ----------------------------------------------------------------------------- : Declaring reflection

/// Declare that a class supports reflection
/** Reflection allows the member variables of a class to be inspected at runtime.
 */
#define DECLARE_REFLECTION_PREFIX(PREFIX,SUFFIX) \
  protected: \
    template<class Handler> void reflect_impl(Handler&); \
    friend class Reader; \
    friend class Writer; \
    friend class GetDefaultMember; \
    friend class GetMember; \
    PREFIX void reflect(Reader& reader) SUFFIX; \
    PREFIX void reflect(Writer& writer) SUFFIX; \
    PREFIX void reflect(GetDefaultMember& gdm) SUFFIX; \
    PREFIX void reflect(GetMember& gm) SUFFIX

#define DECLARE_REFLECTION() DECLARE_REFLECTION_PREFIX(,)

/// Declare that a class supports reflection, which can be overridden in derived classes
#define DECLARE_REFLECTION_VIRTUAL() DECLARE_REFLECTION_PREFIX(virtual,)

#define DECLARE_REFLECTION_OVERRIDE() DECLARE_REFLECTION_PREFIX(,override)

// ----------------------------------------------------------------------------- : Implementing reflection

/// Implement the refelection of a class type Cls
/** Reflection allows the member variables of a class to be inspected at runtime.
 *
 *  Currently creates the methods:
 *  - Reader          ::handle(Cls&)
 *  - Writer          ::handle(const Cls&)
 *  - GetMember       ::handle(const Cls&)
 *  Usage:
 *    @code
 *     IMPLEMENT_REFLECTION(MyClass) {
 *         REFLECT(a_variable_in_my_class);
 *         REFLECT(another_variable_in_my_class);
 *     }
 *    @endcode
 */
#define IMPLEMENT_REFLECTION(Cls) \
  REFLECT_OBJECT_READER(Cls) \
  REFLECT_OBJECT_WRITER(Cls) \
  REFLECT_OBJECT_GET_DEFAULT_MEMBER_NOT(Cls) \
  REFLECT_OBJECT_GET_MEMBER(Cls) \
  template <class Handler> \
  void Cls::reflect_impl(Handler& handler)

/// Implement the refelection of a class type Cls that only uses REFLECT_NAMELESS
#define IMPLEMENT_REFLECTION_NAMELESS(Cls) \
  REFLECT_OBJECT_READER(Cls) \
  REFLECT_OBJECT_WRITER(Cls) \
  REFLECT_OBJECT_GET_DEFAULT_MEMBER(Cls) \
  REFLECT_OBJECT_GET_MEMBER_NOT(Cls) \
  template <class Handler> \
  void Cls::reflect_impl(Handler& handler)

/// Implement the refelection of a class type Cls, but only for Reader and Writer,
/** There is custom code for GetMember and GetDefaultMember */
#define IMPLEMENT_REFLECTION_NO_GET_MEMBER(Cls) \
  REFLECT_OBJECT_READER(Cls) \
  REFLECT_OBJECT_WRITER(Cls) \
  template <class Handler> \
  void Cls::reflect_impl(Handler& handler)

/// Implement the refelection of a class type Cls, but only for Reader and Writer
/** There is no code for GetMember and GetDefaultMember */
#define IMPLEMENT_REFLECTION_NO_SCRIPT(Cls) \
  REFLECT_OBJECT_READER(Cls) \
  REFLECT_OBJECT_WRITER(Cls) \
  REFLECT_OBJECT_GET_DEFAULT_MEMBER_NOT(Cls) \
  REFLECT_OBJECT_GET_MEMBER_NOT(Cls) \
  template <class Handler> \
  void Cls::reflect_impl(Handler& handler)

/// Reflect a variable
#define REFLECT(var)          handler.handle(_(#var), var)
/// Reflect a variable under the given name
#define REFLECT_N(name, var)  handler.handle(_(name), var)
/// Reflect a variable without a name, should be used only once per class
#define REFLECT_NAMELESS(var) handler.handle(var)

/// Declare that the variables of a base class should also be reflected
#define REFLECT_BASE(Base)    Base::reflect_impl(handler)

/// Reflect a group of declarations only when reading
/** Usage:
 *  @code
 *   REFLECT_IF_READING {
 *      // only executed by Reader
 *   }
 *  @endcode
 */
#define REFLECT_IF_READING if (Handler::isReading)

/// Reflect a group of declarations only when *not* reading
/** Usage:
 *  @code
 *   REFLECT_IF_NOT_READING {
 *      // only executed by Writer, GetMember, GetDefaultMember
 *   }
 *  @endcode
 */
#define REFLECT_IF_NOT_READING if (!Handler::isReading)

/// Reflect a group of declarations only when reading and when the value is a single line value
#define REFLECT_IF_READING_SINGLE_VALUE if (Handler::isReading && !handler.isCompound())

#define REFLECT_IF_READING_SINGLE_VALUE_AND(cond) if (Handler::isReading && !handler.isCompound() && cond)

/// Add an alias for backwards compatibility
/** If a key 'name' is encountered in the input file, it is interpreted as 'var' for versions < version
 *  Example:
 *  @code
 *   REFLECT_COMPAT(<300, "style", stylesheet) // prior to 0.3.0 style was used instead of stylesheet
 *  @encode
 */
#define REFLECT_COMPAT(cond, name, var) if (handler.formatVersion() cond) REFLECT_N(name,var)

/// Ignore things for backwards compatibility for versions < 'version'
#define REFLECT_COMPAT_IGNORE(cond, name, Type) if (reflector.formatVersion() cond) {Type ignored; REFLECT_N(name,ignored);}

/// Reflect a variable, ignores the variable for scripting
#define REFLECT_NO_SCRIPT(var)          handler.handleNoScript(_(#var), var)
/// Reflect a variable under the given name
#define REFLECT_NO_SCRIPT_N(name, var)  handler.handleNoScript(_(name), var)

/// Explicitly instantiate reflection; this is occasionally required.
#define INSTANTIATE_REFLECTION(Class) \
  template void Class::reflect_impl<Reader> (Reader&); \
  template void Class::reflect_impl<Writer> (Writer&); \
  template void Class::reflect_impl<GetMember> (GetMember&);

#define INSTANTIATE_REFLECTION_NAMELESS(Class) \
  template void Class::reflect_impl<Reader> (Reader&); \
  template void Class::reflect_impl<Writer> (Writer&); \
  template void Class::reflect_impl<GetDefaultMember> (GetDefaultMember&);

// ----------------------------------------------------------------------------- : Reflecting enums

/// Implement the refelection of a enumeration type Enum
/** Usage:
 *    @code
 *     IMPLEMENT_REFLECTION_ENUM(MyEnum) {
 *         VALUE(value_of_enum_1);
 *         VALUE(value_of_enum_2);
 *     }
 *    @endcode
 *
 *  When reading the first value declared is the default value
 *
 *  Currently creates the methods:
 *   - Reader::handle(Enum&)
 *   - Writer::handle(const Enum&)
 *   - GetDefaultMember::handle(const Enum&)
 */
#define IMPLEMENT_REFLECTION_ENUM(Enum) \
  template <class Handler> \
  void reflect_ ## Enum (Enum& enum_, Handler& handler); \
  REFLECT_ENUM_READER(Enum) \
  REFLECT_ENUM_WRITER(Enum) \
  REFLECT_ENUM_GET_MEMBER(Enum) \
  template <class Handler> \
  void reflect_ ## Enum (Enum& enum_, Handler& handler)

/// Declare a possible value of an enum
#define VALUE(val) handler.handle(_(#val), val, enum_)

/// Declare a possible value of an enum under the given name
#define VALUE_N(name, val) handler.handle(_(name), val, enum_)

