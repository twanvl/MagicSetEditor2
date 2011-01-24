//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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
	/// Is the message (potentially) fatal?
	virtual bool is_fatal() const { return false; }
	
  protected:
	String message; ///< The error message
};


/// Internal errors
class InternalError : public Error {
  public:
	InternalError(const String& str);
	// not all internal errors are fatal, but we had still better warn the user about them.
	virtual bool is_fatal() const { return true; }
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
	ScriptParseError(size_t pos1, size_t pos2, int line, const String& filename, const String& open, const String& close, const String& found);
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

/// "Variable not set"
class ScriptErrorNoVariable : public ScriptError {
  public:
	inline ScriptErrorNoVariable(const String& var) : ScriptError(_("Variable not set: ") + var) {}
};

/// "Can't convert from A to B"
class ScriptErrorConversion : public ScriptError {
  public:
	inline ScriptErrorConversion(const String& a, const String& b)
		: ScriptError(_ERROR_2_("can't convert", a, b)) {}
	inline ScriptErrorConversion(const String& value, const String& a, const String& b)
		: ScriptError(_ERROR_3_("can't convert value", value, a, b)) {}
};

/// "A has no member B"
class ScriptErrorNoMember : public ScriptError {
  public:
	inline ScriptErrorNoMember(const String& type, const String& member)
		: ScriptError(_ERROR_2_("has no member", type, member)) {}
};

// ----------------------------------------------------------------------------- : Error/message handling

/// Should a popup be shown for internal errors?
extern bool show_message_box_for_fatal_errors;
extern bool write_errors_to_cli;

/// Types/levels of (error)messages
enum MessageType
{	MESSAGE_INPUT
,	MESSAGE_OUTPUT
,	MESSAGE_INFO
,	MESSAGE_WARNING
,	MESSAGE_ERROR
,	MESSAGE_FATAL_ERROR

,	MESSAGE_TYPE_MAX
};

/// Queue an (error) message, it can later be retrieved with get_queued_message
/** If the message is a MESSAGE_FATAL_ERROR, and show_message_box_for_fatal_errors==true, then a popup is shown
 */
void queue_message(MessageType type, String const& msg);
/// Handle an error by queuing a message
void handle_error(const Error& e);

/// Are there any queued messages?
bool have_queued_message();

/// Get the first queued message, or return false
bool get_queued_message(MessageType& type, String& msg);


/// Make a stack trace for use in InternalErrors
String get_stack_trace();

/// Catch all types of errors, and pass then to handle_error
#define CATCH_ALL_ERRORS(handle_now) \
	catch (const Error& e) { \
		handle_error(e); \
	} catch (const std::exception& e) { \
		/* we don't throw std::exception ourselfs, so this is probably something serious */ \
		String message(e.what(), IF_UNICODE(wxConvLocal, wxSTRING_MAXLEN) ); \
		handle_error(InternalError(message)); \
	} catch (...) { \
		handle_error(InternalError(_("An unexpected exception occurred!"))); \
	}

// ----------------------------------------------------------------------------- : EOF
#endif
