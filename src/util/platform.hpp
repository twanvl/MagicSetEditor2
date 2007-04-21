//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_PLATFORM
#define HEADER_UTIL_PLATFORM

/** @file util/platform.hpp
 *
 *  @brief Platform and compiler specific hacks.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Windows


// ----------------------------------------------------------------------------- : Linux

#ifdef __linux__
	
	/// wxMkDir as documented
	inline void wxMkDir(const String& dir) {
		wxMkDir(wxConvLocal.cWX2MB(dir), 0777);
	}
	
#endif

// ----------------------------------------------------------------------------- : GCC

#ifdef __GNUC__
	
	/// Absolute value of integers
	template <typename T>
	inline T abs(T a) { return a < 0 ? -a : a; }
	
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
