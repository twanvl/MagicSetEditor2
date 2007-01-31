//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_PLATFORM
#define HEADER_UTIL_PLATFORM

/** @file util/platform.hpp
 *
 *  @brief Platform specific hacks
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Windows


// ----------------------------------------------------------------------------- : Linux

#ifdef __linux__
	
	inline void wxMkDir(const String& dir) {
		wxMkDir(wxConvLocal.cWX2MB(dir), 0777);
	}
	
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
