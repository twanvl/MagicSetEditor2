//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_DYNAMIC_ARG
#define HEADER_UTIL_DYNAMIC_ARG

/** @file util/dynamic_arg.hpp
 *
 *  @brief Support for 'dynamicly scopped' arguments.
 *  This header
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Dynamic argument

#ifdef _MSC_VER
#	define THREAD_LOCAL __declspec(thread)
#else
#	define THREAD_LOCAL __thread
#endif

/// Declare a dynamic argument.
/** The value of the argument can be got with: name()
 *  To change the value use WITH_DYNAMIC_ARG(name, newValue)
 *  To be used in a header file. Use IMPLEMENT_DYN_ARG in a source file
 */
#define DECLARE_DYNAMIC_ARG(Type, name)					\
	extern THREAD_LOCAL Type name##_private;			\
	inline Type name() { return name##_private; }		\
	class name##_changer {								\
	  public:											\
		inline name##_changer(Type const& newValue)		\
			: oldValue(name##_private) {				\
			name##_private = newValue;					\
		}												\
		inline ~name##_changer() {						\
			name##_private = oldValue;					\
		}												\
	  private:											\
		Type oldValue;									\
	}

/// Implementation of a dynamic argument
#define IMPLEMENT_DYNAMIC_ARG(Type, name, initial)		\
	THREAD_LOCAL Type name##_private = initial;

/// Locally change the value of a dynamic argument
/** Usage:
 *  @code
 *   // here name() == old value
 *   {
 *      WITH_DYNAMIC_ARG(name, newValue);
 *      // here name() == newValue
 *   }
 *   // here name() == old value
 *  @endcode
 */
#define WITH_DYNAMIC_ARG(name, value)					\
	name##_changer name##_dummmy(value)

// ----------------------------------------------------------------------------- : EOF
#endif
