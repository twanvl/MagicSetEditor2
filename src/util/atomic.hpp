//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ATOMIC
#define HEADER_UTIL_ATOMIC

/** @file util/atomic.hpp
 *
 *  @brief Provides the type AtomicInt, which is an integer that can be incremented and decremented atomicly
 */

// ----------------------------------------------------------------------------- : Includes

// ----------------------------------------------------------------------------- : AtomicInt : windows

#if defined(__WXMSW__)
	#ifdef _MSC_VER
		extern "C" {
			LONG  __cdecl _InterlockedIncrement(LONG volatile *Addend);
			LONG  __cdecl _InterlockedDecrement(LONG volatile *Addend);
		}
		#pragma intrinsic (_InterlockedIncrement)
		#define InterlockedIncrement _InterlockedIncrement
		#pragma intrinsic (_InterlockedDecrement)
		#define InterlockedDecrement _InterlockedDecrement
	#endif
	
	/// An integer which is equivalent to an AtomicInt, but which doesn't support attomic operations
	typedef LONG AtomicIntEquiv;
	
	/// An integer that can be incremented and decremented atomicly
	class AtomicInt {
	  public:
		AtomicInt(AtomicIntEquiv v) : v(v) {}
		inline operator AtomicIntEquiv() const {
			return v;
		}
		/// Attomicly increments this AtomicInt, returns the new value
		inline AtomicIntEquiv operator ++ () {
			return InterlockedIncrement(&v);
		}
		/// Attomicly decrements this AtomicInt, returns the new value
		inline AtomicIntEquiv operator -- () {
			return InterlockedDecrement(&v);
		}
	  private:
		AtomicIntEquiv    v;  ///< The value
	};
	
	/// We have a fast AtomicInt
	#define HAVE_FAST_ATOMIC
	
// ----------------------------------------------------------------------------- : AtomicInt : GCC
#elif defined(__GNUC__) && !defined(HAVE_GCC_ATOMIC_BUILTINS)
	
	/// An integer which is equivalent to an AtomicInt, but which doesn't support attomic operations
	typedef unsigned int AtomicIntEquiv;
	
	/// An integer that can be incremented and decremented atomicly
	class AtomicInt {
	  public:
		AtomicInt(AtomicIntEquiv v) : v(v) {}
		inline operator AtomicIntEquiv() const {
			return v;
		}
		inline AtomicInt operator ++ () {
			return __sync_add_and_fetch(&v,1);
		}
		inline AtomicInt operator -- () {
			return __sync_sub_and_fetch(&v,1);
		}
	  private:
		AtomicIntEquiv v;
	};
	
	/// We have a fast AtomicInt
	#define HAVE_FAST_ATOMIC
	
// ----------------------------------------------------------------------------- : AtomicInt : portable
#else
	
	/// An integer which is equivalent to an AtomicInt, but which doesn't support attomic operations
	typedef long AtomicIntEquiv;
	
	/// An integer that can be incremented and decremented atomicly
	class AtomicInt {
	  public:
		AtomicInt(AtomicIntEquiv v) : v(v) {}
		AtomicInt(const AtomicInt& i) {
			wxCriticalSectionLocker lock(i.cs);
			v = i.v;
		}
		inline operator AtomicIntEquiv() const {
			return v;
		}
		/// Attomicly increments this AtomicInt, returns the new value
		inline AtomicIntEquiv operator ++ () {
			wxCriticalSectionLocker lock(cs);
			return ++v;
		}
		/// Attomicly decrements this AtomicInt, returns the new value
		inline AtomicIntEquiv operator -- () {
			wxCriticalSectionLocker lock(cs);
			return --v;
		}
	  private:
		AtomicIntEquiv            v;  ///< The value
		mutable wxCriticalSection cs; ///< Critical section protecting v
	};
	
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
