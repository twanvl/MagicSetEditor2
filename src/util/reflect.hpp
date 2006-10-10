//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_REFLECT
#define HEADER_UTIL_REFLECT

/** @file util/reflect.hpp
 *
 *  @brief Reflection of classes, currently reflection is used for (de)serialization.
 */

// ----------------------------------------------------------------------------- : Includes

#include "io/reader.hpp"
#include "io/writer.hpp"
#include "io/get_member.hpp"

// ----------------------------------------------------------------------------- : Declaring reflection

/// Declare that a class supports reflection
/** Reflection allows the member variables of a class to be inspected at runtime.
 */
#define DECLARE_REFLECTION()											\
          protected:													\
			template<class Tag> void reflect_impl(Tag& tag);			\
			friend class Reader;										\
			friend class Writer;										\
			friend class GetMember;										\
			void reflect(Reader& reader);								\
			void reflect(Writer& writer);								\
			void reflect(GetMember& getMember)

/// Declare that a class supports reflection, which can be overridden in derived classes
#define DECLARE_REFLECTION_VIRTUAL()									\
          protected:													\
			template<class Tag> void reflect_impl(Tag& tag);			\
			friend class Reader;										\
			friend class Writer;										\
			friend class GetMember;										\
			virtual void reflect(Reader& reader);						\
			virtual void reflect(Writer& writer);						\
			virtual void reflect(GetMember& getMember)

// ----------------------------------------------------------------------------- : Implementing reflection

/// Implement the refelection of a class type Cls
/** Reflection allows the member variables of a class to be inspected at runtime.
 *
 *  Currently creates the methods:
 *  - Reader::handle(Cls&)
 *  - Writer::handle(Cls&)
 *  - GetMember::handle(Cls&)
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
			REFLECT_OBJECT_GET_MEMBER(Cls)								\
			/* Extra level, so it can be declared virtual */			\
			void Cls::reflect(Reader& reader) {							\
				reflect_impl(reader);									\
			}															\
			void Cls::reflect(Writer& writer) {							\
				reflect_impl(writer);									\
			}															\
			void Cls::reflect(GetMember& getMember) {					\
				reflect_impl(getMember);								\
			}															\
			template <class Tag>										\
			void Cls::reflect_impl(Tag& tag)

/// Reflect a variable
#define REFLECT(var)          tag.handle(_(#var), var)
/// Reflect a variable under the given name
#define REFLECT_N(name, var)  tag.handle(_(name), var)

/// Declare that the variables of a base class should also be reflected
#define REFLECT_BASE(Base)    Base::reflect_impl(tag)


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
 *   - GetMember::handle(const Enum&)
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
