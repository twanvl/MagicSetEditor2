//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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

// Std headers
#include <vector>
#include <map>
#include <set>
using namespace std;

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

// ----------------------------------------------------------------------------- : EOF
#endif
