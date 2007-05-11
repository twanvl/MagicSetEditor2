//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_REFLECT
#define HEADER_UTIL_REFLECT

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
#define DECLARE_REFLECTION()											\
          protected:													\
			template<class Tag> void reflect_impl(Tag& tag);			\
			friend class Reader;										\
			friend class Writer;										\
			friend class GetDefaultMember;								\
			friend class GetMember;										\
			void reflect(Reader& reader);								\
			void reflect(Writer& writer);								\
			void reflect(GetDefaultMember& gdm);						\
			void reflect(GetMember& gm)

/// Declare that a class supports reflection, which can be overridden in derived classes
#define DECLARE_REFLECTION_VIRTUAL()									\
          protected:													\
			template<class Tag> void reflect_impl(Tag& tag);			\
			friend class Reader;										\
			friend class Writer;										\
			friend class GetDefaultMember;								\
			friend class GetMember;										\
			/* extra level of indirection between Tag::handle			\
			 * and reflect_impl, to allow for virtual */				\
			virtual void reflect(Reader& reader);						\
			virtual void reflect(Writer& writer);						\
			virtual void reflect(GetDefaultMember& gdm);				\
			virtual void reflect(GetMember& gm)

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
#define IMPLEMENT_REFLECTION(Cls)										\
			REFLECT_OBJECT_READER(Cls)									\
			REFLECT_OBJECT_WRITER(Cls)									\
			REFLECT_OBJECT_GET_DEFAULT_MEMBER_NOT(Cls)					\
			REFLECT_OBJECT_GET_MEMBER(Cls)								\
			template <class Tag>										\
			void Cls::reflect_impl(Tag& tag)

/// Implement the refelection of a class type Cls that only uses REFLECT_NAMELESS
#define IMPLEMENT_REFLECTION_NAMELESS(Cls)								\
			REFLECT_OBJECT_READER(Cls)									\
			REFLECT_OBJECT_WRITER(Cls)									\
			REFLECT_OBJECT_GET_DEFAULT_MEMBER(Cls)						\
			REFLECT_OBJECT_GET_MEMBER_NOT(Cls)							\
			template <class Tag>										\
			void Cls::reflect_impl(Tag& tag)

/// Implement the refelection of a class type Cls, but only for Reader and Writer,
/** There is custom code for GetMember and GetDefaultMember */
#define IMPLEMENT_REFLECTION_NO_GET_MEMBER(Cls)							\
			REFLECT_OBJECT_READER(Cls)									\
			REFLECT_OBJECT_WRITER(Cls)									\
			template <class Tag>										\
			void Cls::reflect_impl(Tag& tag)

/// Implement the refelection of a class type Cls, but only for Reader and Writer
/** There is no code for GetMember and GetDefaultMember */
#define IMPLEMENT_REFLECTION_NO_SCRIPT(Cls)								\
			REFLECT_OBJECT_READER(Cls)									\
			REFLECT_OBJECT_WRITER(Cls)									\
			REFLECT_OBJECT_GET_DEFAULT_MEMBER_NOT(Cls)					\
			REFLECT_OBJECT_GET_MEMBER_NOT(Cls)							\
			template <class Tag>										\
			void Cls::reflect_impl(Tag& tag)

/// Reflect a variable
#define REFLECT(var)          tag.handle(_(#var), var)
/// Reflect a variable under the given name
#define REFLECT_N(name, var)  tag.handle(_(name), var)
/// Reflect a variable without a name, should be used only once per class
#define REFLECT_NAMELESS(var) tag.handle(var)

/// Declare that the variables of a base class should also be reflected
#define REFLECT_BASE(Base)    Base::reflect_impl(tag)

/// Reflect a group of declarations only when reading
/** Usage:
 *  @code
 *   REFLECT_IF_READING {
 *      // only executed by Reader
 *   }
 *  @endcode
 */
#define REFLECT_IF_READING    if (tag.reading())

/// Reflect a group of declarations only when *not* reading
/** Usage:
 *  @code
 *   REFLECT_IF_NOT_READING {
 *      // only executed by Writer, GetMember, GetDefaultMember
 *   }
 *  @endcode
 */
#define REFLECT_IF_NOT_READING  if (!tag.reading())

/// Add an alias for backwards compatability
/** If a key 'old' is encountered in the input file, it is interpreted as 'new' for versions < version
 *  Example:
 *  @code
 *   REFLECT_ALIAS(300, "style", "stylesheet") // prior to 0.3.0 style was used instead of stylesheet
 *  @encode
 */
#define REFLECT_ALIAS(version, old, new) tag.addAlias(version, _(old), _(new))

/// Reflect a variable, ignores the variable for scripting
#define REFLECT_NO_SCRIPT(var)          tag.handleNoScript(_(#var), var)
/// Reflect a variable under the given name
#define REFLECT_NO_SCRIPT_N(name, var)  tag.handleNoScript(_(name), var)

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
 *   - Reader::handle(Enum&
 *   - Writer::handle(const Enum&)
 *   - GetDefaultMember::handle(const Enum&)
 */
#define IMPLEMENT_REFLECTION_ENUM(Enum)									\
			template <class Tag>										\
			void reflect_ ## Enum (Enum& enum_, Tag& tag);				\
			REFLECT_ENUM_READER(Enum)									\
			REFLECT_ENUM_WRITER(Enum)									\
			REFLECT_ENUM_GET_MEMBER(Enum)								\
			template <class Tag>										\
			void reflect_ ## Enum (Enum& enum_, Tag& tag)

/// Declare a possible value of an enum
#define VALUE(val)			tag.handle(_(#val), val, enum_)

/// Declare a possible value of an enum under the given name
#define VALUE_N(name, val)	tag.handle(_(name), val, enum_)

// ----------------------------------------------------------------------------- : EOF
#endif
