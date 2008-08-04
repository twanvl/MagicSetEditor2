//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_CLI_TEXT_IO_HANDLER
#define HEADER_CLI_TEXT_IO_HANDLER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Text I/O handler

// color codes
extern const Char *BRIGHT, *NORMAL, *PARAM, *FILE_EXT, *GRAY, *RED, *ENDL;

/// Command line input / output handler
class TextIOHandler {
  public:
	void init();
	
	/// Do we have a console to read/write from/to?
	bool haveConsole() const;
	
	/// Output text to the console
	TextIOHandler& operator << (const Char*);
	TextIOHandler& operator << (const String&);
	
	/// Read a line from stdin
	String getLine();
	/// Can another line be got?
	bool canGetLine();
	
	/// Flush output
	void flush();
	
	/// Show an error message
	void showError(const String& message);
	/// Show a warning message
	void showWarning(const String& message);
	
	/// Enable raw mode
	void enableRaw();
	/// Output a single raw-mode record
	/// Has no effect unless enableRaw() was called
	void flushRaw();
	
  private:
	bool have_console;
	bool escapes;
	FILE* stream;
	String buffer; ///< Buffer when not writing to console
	bool raw_mode;
	int raw_mode_status;
};

/// The global TextIOHandler object
extern TextIOHandler cli;

// ----------------------------------------------------------------------------- : EOF
#endif
