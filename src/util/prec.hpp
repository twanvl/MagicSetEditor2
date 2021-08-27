//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file util/prec.hpp
 *
 *  @brief Precompiled header, and aliasses for common types
 */

// ----------------------------------------------------------------------------- : Compiler specific

#ifdef _MSC_VER
#  pragma warning (disable: 4100) // unreferenced formal parameter
#  pragma warning (disable: 4355) // 'this' : used in base member initializer list
#  pragma warning (disable: 4800) // 'int' : forcing value to bool 'true' or 'false' (performance warning)
#  pragma warning (disable: 4675) // resolved overload was found by argument-dependent lookup (occurs in some boost header)
#  define _CRT_NO_VA_START_VALIDATION // fix "va_start argument must not have reference type and must not be parenthesized"
#endif

// ----------------------------------------------------------------------------- : Includes

// Wx headers
#include <wx/setup.h>
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/datetime.h>
#include <wx/regex.h> // TODO : remove, see regex.hpp

#if defined(__WXMSW__)
  // MSW uses the RGB define, fix it before it's undefined 
  #include <wx/msw/private.h>
#endif

// Std headers
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
using namespace std;

#undef RGB

// ----------------------------------------------------------------------------- : Wx Aliasses

// Remove some of the wxUglyness

typedef wxWindow Window;

typedef wxBitmap Bitmap;
typedef wxImage  Image;
typedef wxDC     DC;

typedef wxDateTime DateTime;

typedef wxOutputStream OutputStream;

// ----------------------------------------------------------------------------- : Compatability fixes

#if wxVERSION_NUMBER < 2805
  #define wxBORDER_THEME wxSUNKEN_BORDER
#endif
#if wxVERSION_NUMBER < 2900
  // wx >= 2.9 requires the use of HandleWindowEvent on windows, instead of ProcessEvent
  #define HandleWindowEvent ProcessEvent
#endif
#if wxVERSION_NUMBER < 2700
  // is it worth it to still support wx2.6?
  #define wxFD_SAVE             wxSAVE
  #define wxFD_OPEN             wxOPEN
  #define wxFD_OVERWRITE_PROMPT wxOVERWRITE_PROMPT
  #define SetDeviceClippingRegion SetClippingRegion
  typedef wxEvent wxMouseCaptureLostEvent;
  #define EVT_MOUSE_CAPTURE_LOST(handler) // ignore
  #define wxEVT_MOUSE_CAPTURE_LOST 12345678 // not an actual event type
  #define wxAutoBufferedPaintDC wxBufferedPaintDC
#endif

// ----------------------------------------------------------------------------- : Other aliasses

typedef unsigned char Byte;
typedef unsigned int  UInt;

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

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
  // Use OutputDebugString/DebugBreak for assertions if in debug mode
  void msvc_assert(const wchar_t*, const wchar_t*, const wchar_t*, unsigned);
  #undef assert
  #define assert(exp) (void)( (exp) || (msvc_assert(nullptr, _CRT_WIDE(#exp), _CRT_WIDE(__FILE__), __LINE__), 0) )
#endif

#ifdef __APPLE__
  #if wxVERSION_NUMBER < 3100
  // wx <= 3.1 doesn't include a hash implementation for wxString
  template <> struct std::hash<wxString> {
    size_t operator()(const wxString& k) const {
      return hash<wstring>()(k.ToStdWstring());
    }
  };
  #endif
#endif
