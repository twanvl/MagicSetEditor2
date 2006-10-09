//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ERROR
#define HEADER_UTIL_ERROR

/** @file util/error.hpp
 *
 *  @brief Classes and functions for handling errors/exceptions.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Error types

/// Our own exception class
class Error {
  public:
	Error(const String& message);
	virtual ~Error();
	
	/// Return the error message
	virtual String what() const; 
	
  private:
	String message; ///< The error message
};


/// Internal errors
class InternalError : public Error {
  public:
	inline InternalError(const String& str)
		: Error(_("An internal error occured, please contact the author:\n") + str)
	{}
};

// ----------------------------------------------------------------------------- : File errors

/// Errors related to packages
class PackageError : public Error {
  public:
	inline PackageError(const String& str) : Error(str) {}
};

/// A file is not found
class FileNotFoundError : public PackageError {
  public:
	inline FileNotFoundError(const String& file, const String& package)
		: PackageError(_("File not found: ") + file + _(" in package ") + package)
	{}
};

// ----------------------------------------------------------------------------- : Parse errors

/// Parse errors
class ParseError : public Error {
  public:
	inline ParseError(const String& str) : Error(str) {}
};

// ----------------------------------------------------------------------------- : EOF
#endif
