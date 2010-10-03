//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_SMART_PTR
#define HEADER_UTIL_SMART_PTR

/** @file util/smart_ptr.hpp
 *
 *  @brief Utilities related to boost smart pointers
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/atomic.hpp>
#ifdef HAVE_FAST_ATOMIC
	/// Using intrusive_ptr where possible? (as opposed to smart_ptr)
	#define USE_INTRUSIVE_PTR
#endif

// Use slightly less fancy template stuff, so msvc7.1 doesn't crash with an internal compiler error
#define BOOST_SP_NO_SP_CONVERTIBLE

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#ifdef USE_INTRUSIVE_PTR
	#include <boost/intrusive_ptr.hpp>
#endif

// Can't do  using namespace boost;
// because boost::shared_ptr conflicts with std::tr1::shared_ptr
// and some boost headers do include boost/shared_ptr themselves
#if _HAS_TR1
	using std::tr1::shared_ptr;
#else
	using boost::shared_ptr;
#endif
using boost::intrusive_ptr;
using boost::scoped_ptr;
using boost::static_pointer_cast;
using boost::dynamic_pointer_cast;

// ----------------------------------------------------------------------------- : Declaring

/// Declares the type TypeP as a shared_ptr<Type>
#define DECLARE_SHARED_POINTER_TYPE(Type)	\
	class Type;								\
	typedef shared_ptr<Type> Type##P;

// ----------------------------------------------------------------------------- : Creating

/// Wrap a newly allocated pointer in an shared_ptr
/** Usage:
	*    return shared(new T(stuff)));
	*/
template <typename T>
inline shared_ptr<T> shared(T* ptr) {
	return shared_ptr<T>(ptr);
}

// ----------------------------------------------------------------------------- : Intrusive pointers

#ifdef USE_INTRUSIVE_PTR

	/// Declares the type TypeP as a intrusive_ptr<Type>
	#define DECLARE_POINTER_TYPE(Type)			\
		class Type;								\
		typedef intrusive_ptr<Type> Type##P;

	/// Wrap a newly allocated pointer in an intrusive_ptr
	/** Usage:
	 *    return intrusive(new T(stuff)));
	 */
	template <typename T>
	inline intrusive_ptr<T> intrusive(T* ptr) {
		return intrusive_ptr<T>(ptr);
	}

	// ----------------------------------------------------------------------------- : Intrusive pointer base
	
	template <typename T> class IntrusivePtrBase;
	template <typename T> void intrusive_ptr_add_ref(IntrusivePtrBase<T>*);
	template <typename T> void intrusive_ptr_release(IntrusivePtrBase<T>*);
	/// Base class for objects wishing to use intrusive_ptrs.
	/** There is no implicit virtual destructor, objects are destructed as type T
	*   Usage:
	 *  @code
	 *    DECLARE_POINTER_TYPE(MyClass);
	 *    class MyClass : public IntrusivePtrBase<MyClass> { ... }
	 *  @endcode
	 */
	template <typename T> class IntrusivePtrBase {
	  public:
		inline IntrusivePtrBase()                        : ref_count(0) {}
		// don't copy construct the reference count!
		inline IntrusivePtrBase(const IntrusivePtrBase&) : ref_count(0) {}
		// don't assign the reference count!
		inline void operator = (const IntrusivePtrBase&) { }
	  protected:
		/// Delete this object, can be overloaded
		inline void destroy() {
			delete static_cast<T*>(this);
		}
	  private:
		AtomicInt ref_count;
		friend void intrusive_ptr_add_ref <> (IntrusivePtrBase*);
		friend void intrusive_ptr_release <> (IntrusivePtrBase*);
	};

	template <typename T> void intrusive_ptr_add_ref(IntrusivePtrBase<T>* p) {
		++(p->ref_count);
	}
	
	template <typename T> void intrusive_ptr_release(IntrusivePtrBase<T>* p) {
		if (--p->ref_count == 0) {
			static_cast<T*>(p)->destroy();
		}
	}
	// ----------------------------------------------------------------------------- : Intrusive pointer base : virtual
	
	/// IntrusivePtrBase with a virtual destructor
	class IntrusivePtrVirtualBase : public IntrusivePtrBase<IntrusivePtrVirtualBase> {
	  public:
		virtual ~IntrusivePtrVirtualBase() {}
	};
	
	// ----------------------------------------------------------------------------- : Intrusive pointer base : with delete
	
	/// Base class for objects wishing to use intrusive_ptrs, using a manual delete function
	class IntrusivePtrBaseWithDelete : public IntrusivePtrBase<IntrusivePtrBaseWithDelete> {
	  public:
		virtual ~IntrusivePtrBaseWithDelete() {}
	  protected:
		/// Delete this object
		virtual void destroy() {
			delete this;
		}
		template <typename T> friend void intrusive_ptr_release(IntrusivePtrBase<T>*);
	};
	
#else
	#define DECLARE_POINTER_TYPE DECLARE_SHARED_POINTER_TYPE
	#define intrusive_ptr shared_ptr
	
	template <typename T> class IntrusivePtrBase {};
	
	/// IntrusivePtrBase with a virtual destructor
	class IntrusivePtrVirtualBase : public IntrusivePtrBase<IntrusivePtrVirtualBase> {
	  public:
		virtual ~IntrusivePtrVirtualBase() {}
	};
	
	class IntrusivePtrBaseWithDelete : public IntrusivePtrBase<IntrusivePtrBaseWithDelete> {
	  public:
		virtual ~IntrusivePtrBaseWithDelete() {}
	  protected:
		/// Delete this object
		virtual void destroy() {
			delete this;
		}
	};
	
#endif

/// Pointer to 'anything'
typedef intrusive_ptr<IntrusivePtrVirtualBase> VoidP;

// ----------------------------------------------------------------------------- : EOF
#endif
