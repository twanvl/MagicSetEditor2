//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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
#include <wx/thread.h>

// ----------------------------------------------------------------------------- : Dynamic argument

#ifdef _MSC_VER
#	define THREAD_LOCAL __declspec(thread)
#	define HAVE_TLS 1
#elif defined __linux
#	define THREAD_LOCAL __thread
#	define HAVE_TLS 1
#else
#	define HAVE_TLS 0
#endif

#if HAVE_TLS

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

#else

	template <typename T> struct ThreadLocalObject {
		map<int,T*> objects;
		T def;
		wxCriticalSection container_access;

		inline T operator () () {
			wxCriticalSectionLocker lock(container_access);
			T*& p = objects[wxThread::GetCurrentId()];
			if (!p) {
				p = new T(def);
			}
			return *objects[wxThread::GetCurrentId()];
		}

		inline void store (T value) {
			wxCriticalSectionLocker lock(container_access);
			T*& p = objects[wxThread::GetCurrentId()];
			if (!p) {
				p = new T(def);
			}
			*objects[wxThread::GetCurrentId()] = value;
		}

		ThreadLocalObject (T def)
		: def(def)
		{}

		~ThreadLocalObject () {
			for (typename map<int,T*>::iterator i = objects.begin(); i != objects.end(); ++i) {
				delete (*i).second;
			}
		}
	};
	
	#define DECLARE_DYNAMIC_ARG(Type, name)				\
		extern ThreadLocalObject<Type> name;

	#define IMPLEMENT_DYNAMIC_ARG(Type, name, initial)	\
		ThreadLocalObject<Type> name (initial);

	#define WITH_DYNAMIC_ARG(name, value)				\
		name.store(value);

#endif

// ----------------------------------------------------------------------------- : EOF
#endif
