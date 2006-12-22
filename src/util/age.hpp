//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_AGE
#define HEADER_UTIL_AGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/dynamic_arg.hpp>

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

// ----------------------------------------------------------------------------- : Age

/// Represents the age of a value, higher values are newer
/** Age is counted using a global variable */
class Age {
  public:
	/// Construct a new age value, 
	Age() {
		update();
	}
	Age(LONG age) : age(age) {}
	
	/// Update the age to become the newest one
	inline void update() {
		age = InterlockedIncrement(&new_age);
	}
	
	/// Compare two ages, smaller means earlier
	inline bool operator < (Age a) const { return age < a.age; }
	
	/// A number corresponding to the age
	inline LONG get() const { return age; }
	
  private:
	/// This age
	LONG age;
	/// Global age counter, value of the last age created
	static volatile LONG new_age;
};


/// Age the object currently being processed was last updated
/** NOTE:
 *  image generating functions have two modes
 *  if last_update_age >  0 they return whether the image is still up to date
 *  if last_update_age == 0 they generate the image
 */
DECLARE_DYNAMIC_ARG  (long, last_update_age);

// ----------------------------------------------------------------------------- : EOF
#endif
