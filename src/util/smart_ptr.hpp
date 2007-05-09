//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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

#include <boost/shared_ptr.hpp>
#ifdef USE_INTRUSIVE_PTR
	#include <boost/intrusive_ptr.hpp>
#endif
using namespace boost;

// ----------------------------------------------------------------------------- : Declaring

/// Declares the type TypeP as a shared_ptr<Type>
#define DECLARE_POINTER_TYPE(Type)			\
	class Type;								\
	typedef shared_ptr<Type> Type##P;

// ----------------------------------------------------------------------------- : Creating

/// Allocate a new shared-pointed object
template <typename T>
inline shared_ptr<T> new_shared() {
	return shared_ptr<T>(new T());
}
/// Allocate a new shared-pointed object, given one argument to pass to the ctor of T
template <typename T, typename A0>
inline shared_ptr<T> new_shared1(const A0& a0) {
	return shared_ptr<T>(new T(a0));
}
/// Allocate a new shared-pointed object, given two arguments to pass to the ctor of T
template <typename T, typename A0, typename A1>
inline shared_ptr<T> new_shared2(const A0& a0, const A1& a1) {
	return shared_ptr<T>(new T(a0, a1));
}
/// Allocate a new shared-pointed object, given three arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2>
inline shared_ptr<T> new_shared3(const A0& a0, const A1& a1, const A2& a2) {
	return shared_ptr<T>(new T(a0, a1, a2));
}
/// Allocate a new shared-pointed object, given four arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2, typename A3>
inline shared_ptr<T> new_shared4(const A0& a0, const A1& a1, const A2& a2, const A3& a3) {
	return shared_ptr<T>(new T(a0, a1, a2, a3));
}
/// Allocate a new shared-pointed object, given five arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
inline shared_ptr<T> new_shared5(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4) {
	return shared_ptr<T>(new T(a0, a1, a2, a3, a4));
}
/// Allocate a new shared-pointed object, given six arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
inline shared_ptr<T> new_shared6(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) {
	return shared_ptr<T>(new T(a0, a1, a2, a3, a4, a5));
}
/// Allocate a new shared-pointed object, given seven arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline shared_ptr<T> new_shared7(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) {
	return shared_ptr<T>(new T(a0, a1, a2, a3, a4, a5, a6));
}
/// Allocate a new shared-pointed object, given eight arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline shared_ptr<T> new_shared8(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) {
	return shared_ptr<T>(new T(a0, a1, a2, a3, a4, a5, a6, a7));
}
/// Allocate a new shared-pointed object, given nine arguments to pass to the ctor of T
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline shared_ptr<T> new_shared9(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) {
	return shared_ptr<T>(new T(a0, a1, a2, a3, a4, a5, a6, a7, a8));
}

// ----------------------------------------------------------------------------- : Intrusive pointers

#ifdef USE_INTRUSIVE_PTR

	/// Declares the type TypeP as a intrusive_ptr<Type>
	#define DECLARE_INTRUSIVE_POINTER_TYPE(Type)		\
		class Type;										\
		typedef intrusive_ptr<Type> Type##P;


	/// Allocate a new intrusive-pointed object
	template <typename T>
	inline intrusive_ptr<T> new_intrusive() {
		return intrusive_ptr<T>(new T());
	}
	/// Allocate a new intrusive-pointed object, given one argument to pass to the ctor of T
	template <typename T, typename A0>
	inline intrusive_ptr<T> new_intrusive1(const A0& a0) {
		return intrusive_ptr<T>(new T(a0));
	}
	/// Allocate a new intrusive-pointed object, given two arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1>
	inline intrusive_ptr<T> new_intrusive2(const A0& a0, const A1& a1) {
		return intrusive_ptr<T>(new T(a0, a1));
	}
	/// Allocate a new intrusive-pointed object, given three arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2>
	inline intrusive_ptr<T> new_intrusive3(const A0& a0, const A1& a1, const A2& a2) {
		return intrusive_ptr<T>(new T(a0, a1, a2));
	}
	/// Allocate a new intrusive-pointed object, given four arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2, typename A3>
	inline intrusive_ptr<T> new_intrusive4(const A0& a0, const A1& a1, const A2& a2, const A3& a3) {
		return intrusive_ptr<T>(new T(a0, a1, a2, a3));
	}
	/// Allocate a new intrusive-pointed object, given five arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
	inline intrusive_ptr<T> new_intrusive5(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4) {
		return intrusive_ptr<T>(new T(a0, a1, a2, a3, a4));
	}
	/// Allocate a new intrusive-pointed object, given six arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline intrusive_ptr<T> new_intrusive6(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) {
		return intrusive_ptr<T>(new T(a0, a1, a2, a3, a4, a5));
	}
	/// Allocate a new intrusive-pointed object, given seven arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline intrusive_ptr<T> new_intrusive7(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) {
		return intrusive_ptr<T>(new T(a0, a1, a2, a3, a4, a5, a6));
	}
	/// Allocate a new intrusive-pointed object, given eight arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	inline intrusive_ptr<T> new_intrusive8(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) {
		return intrusive_ptr<T>(new T(a0, a1, a2, a3, a4, a5, a6, a7));
	}
	/// Allocate a new intrusive-pointed object, given nine arguments to pass to the ctor of T
	template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	inline intrusive_ptr<T> new_intrusive9(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) {
		return intrusive_ptr<T>(new T(a0, a1, a2, a3, a4, a5, a6, a7, a8));
	}
	
	/// Base class for objects wishing to use intrusive_ptrs
	class IntrusivePtrBase {
	  public:
		inline IntrusivePtrBase() : ref_count(0) {}
		virtual ~IntrusivePtrBase() {}
	  protected:
		/// Delete this object
		virtual void destroy() {
			delete this;
		}
	  private:
		AtomicInt ref_count;
		friend void intrusive_ptr_add_ref(IntrusivePtrBase*);
		friend void intrusive_ptr_release(IntrusivePtrBase*);
	};
	
	inline void intrusive_ptr_add_ref(IntrusivePtrBase* p) {
		++p->ref_count;
	}
	inline void intrusive_ptr_release(IntrusivePtrBase* p) {
		if (--p->ref_count == 0) {
			p->destroy();
		}
	}
	
#else
	#define DECLARE_INTRUSIVE_POINTER_TYPE DECLARE_POINTER_TYPE
	#define intrusive_ptr shared_ptr
	#define new_intrusive  new_shared
	#define new_intrusive1 new_shared1
	#define new_intrusive2 new_shared2
	#define new_intrusive3 new_shared3
	#define new_intrusive4 new_shared4
	#define new_intrusive5 new_shared5
	#define new_intrusive6 new_shared6
	#define new_intrusive7 new_shared7
	#define new_intrusive8 new_shared8
	#define new_intrusive9 new_shared9
	
	class IntrusivePtrBase {
	  public:
		virtual ~IntrusivePtrBase() {};
	  protected:
		/// Delete this object
		virtual void destroy() {
			delete this;
		}
	};
	
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
