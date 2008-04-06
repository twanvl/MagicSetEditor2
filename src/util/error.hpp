//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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
		: Error(_ERROR_1_("internal error", str.c_str()))
	{}
};

// ----------------------------------------------------------------------------- : File errors

/// Errors related to packages
class PackageError : public Error {
  public:
	inline PackageError(const String& str) : Error(str) {}
};

/// A package is not found
class PackageNotFoundError : public PackageError {
  public:
	inline PackageNotFoundError(const String& str) : PackageError(str) {}
};

/// A file is not found
class FileNotFoundError : public PackageError {
  public:
	inline FileNotFoundError(const String& file, const String& package)
		: PackageError(_ERROR_2_("file not found", file, package))
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
		ParseError(_ERROR_2_("file parse error", file, err))
	{}
};

/// Parse error in a script
class ScriptParseError : public ParseError {
  public:
	ScriptParseError(size_t pos, int line, const String& filename, const String& str);
	ScriptParseError(size_t pos, int line, const String& filename, const String& expected, const String& found);
	/// Position of the error
	size_t start, end;
	/// Line number of the error (the first line is 1)
	int line;
	/// Filename the error was in, or an empty string
	String filename;
	/// Return the error message
	virtual String what() const; 
};

/// Multiple parse errors in a script
class ScriptParseErrors : public ParseError {
  public:
	ScriptParseErrors(const vector<ScriptParseError>& errors);
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

/// Handle a warning by showing a message box
void handle_warning(const String& w, bool now = true);

/// Handle errors and warnings that were not handled immediatly in handleError
/** Should be called repeatedly (e.g. in an onIdle event handler) */
void handle_pending_errors();

// ----------------------------------------------------------------------------- : EOF
#endif
