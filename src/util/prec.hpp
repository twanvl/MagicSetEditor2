//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_PREC
#define HEADER_UTIL_PREC

/** @file util/prec.hpp
 *
 *  @brief Precompiled header, and aliasses for common types
 */

// ----------------------------------------------------------------------------- : Compiler specific

#ifdef _MSC_VER
#	pragma warning (disable: 4100) // unreferenced formal parameter
#	pragma warning (disable: 4355) // 'this' : used in base member initializer list
#	pragma warning (disable: 4800) // 'int' : forcing value to bool 'true' or 'false' (performance warning)
#endif

// ----------------------------------------------------------------------------- : Includes

// Wx headers
#include <wx/setup.h>
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/datetime.h>
#include <wx/regex.h> // TODO : remove, see regex.hpp

// Std headers
#include <vector>
#include <map>
#include <set>
using namespace std;

#undef RGB

// ----------------------------------------------------------------------------- : Wx Aliasses

// Remove some of the wxUglyness

typedef wxPanel  Panel;
typedef wxWindow Window;
typedef wxFrame  Frame;

typedef wxBitmap Bitmap;
typedef wxImage  Image;
typedef wxColour Color;
typedef wxDC     DC;

typedef wxDateTime DateTime;

typedef wxOutputStream OutputStream;

// ----------------------------------------------------------------------------- : Other aliasses

typedef unsigned char Byte;
typedef unsigned int  UInt;

/// Null pointer
#define nullptr 0

/// A string standing for a filename, has different behaviour when reading/writing
class FileName : public wxString {
  public:
	FileName()                                {}
	FileName(const wxString& s) : wxString(s) {}
};

// ----------------------------------------------------------------------------- : MSE Headers

// MSE utility headers (ones unlikely to change and used everywhere)
#include "for_each.hpp"
#include "string.hpp"
#include "smart_ptr.hpp"
#include "index_map.hpp"
#include "locale.hpp"
#include "error.hpp"
#include "reflect.hpp"
#include "regex.hpp"

// ----------------------------------------------------------------------------- : Debugging fixes

#ifdef _MSC_VER
	//# pragma conform(forScope,on)    // in "for(int x=..);" x goes out of scope after the for
	// somehow forScope pragma doesn't work in precompiled headers, use this hack instead:
	#ifdef _DEBUG
		#define for if(false);else for
	#endif
	
	#ifdef _DEBUG
		// Use OutputDebugString/DebugBreak for assertions if in debug mode
		void msvc_assert(const wchar_t*, const wchar_t*, const wchar_t*, unsigned);
		#undef assert
		#define assert(exp) (void)( (exp) || (msvc_assert(nullptr, _CRT_WIDE(#exp), _CRT_WIDE(__FILE__), __LINE__), 0) )
	#endif
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
