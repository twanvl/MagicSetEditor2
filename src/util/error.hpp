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

/// Parse error in a particular file
class FileParseError : public ParseError {
  public:
	inline FileParseError(const String& err, const String& file) :
		ParseError(_("Error while parsing file '") + file + _("':\n") + err)
	{}
};

/// Parse error in a script
class ScriptParseError : public ParseError {
  public:
	inline ScriptParseError(const String& str) : ParseError(str) {}
	inline ScriptParseError(const String& exp, const String& found)
		: ParseError(_("Expected '") + exp + _("' instead of '") + found + _("'")) {}
};

// ----------------------------------------------------------------------------- : Script errors

/// A runtime error in a script
class ScriptError : public Error {
  public:
	inline ScriptError(const String& str) : Error(str) {}
};

// ----------------------------------------------------------------------------- : Error handling

/// Handle an error by showing a message box
/** If !allow_duplicate and the error is the same as the previous error, does nothing.
 *  If !now the error is handled by a later call to handle_pending_errors()
 */
void handle_error(const Error& e, bool allow_duplicate = true, bool now = true);

/// Handle errors that were not handled immediatly in handleError
/** Should be called repeatedly (e.g. in an onIdle event handler) */
void handle_pending_errors();

// ----------------------------------------------------------------------------- : EOF
#endif
