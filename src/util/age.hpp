//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_AGE
#define HEADER_UTIL_AGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/dynamic_arg.hpp>
#include <util/atomic.hpp>

// ----------------------------------------------------------------------------- : Age

/// Represents the age of a value, higher values are newer
/** Age is counted using a global variable */
class Age {
  public:
	/// Construct a new age value
	Age() {
		update();
	}
	/// Create a special age
	/** 0: dummy value, used for other purposes
	 *  1: before 'beginning of time', the age conceptually just before program start
	 *  2..: normal ages
	 */
	Age(AtomicIntEquiv age) : age(age) {}
	
	/// Update the age to become the newest one
	inline void update() {
		age = ++new_age;
	}
	
	/// Compare two ages, smaller means earlier
	inline bool operator < (Age a) const { return age < a.age; }
	/// Compare two ages
	inline bool operator == (Age a) const { return age == a.age; }
	
	/// A number corresponding to the age
	inline AtomicIntEquiv get() const { return age; }
	
  private:
	/// This age
	AtomicIntEquiv age;
	/// Global age counter, value of the last age created
	static AtomicInt new_age;
};


/// Age the object currently being processed was last updated
/** NOTE:
 *  image generating functions have two modes
 *  if last_update_age >  0 they return whether the image is still up to date
 *  if last_update_age == 0 they generate the image
 */
DECLARE_DYNAMIC_ARG  (AtomicIntEquiv, last_update_age);

// ----------------------------------------------------------------------------- : EOF
#endif
