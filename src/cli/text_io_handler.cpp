//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <cli/text_io_handler.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Text I/O handler

const Char* BRIGHT   = _("\x1B[1m");
const Char* NORMAL   = _("\x1B[0m");
const Char* PARAM    = _("\x1B[33m");
const Char* FILE_EXT = _("\x1B[0;1m");
const Char* GRAY     = _("\x1B[1;30m");
const Char* RED      = _("\x1B[1;31m");
const Char* YELLOW   = _("\x1B[1;33m");
const Char* ENDL     = _("\n");

TextIOHandler cli;

#ifdef __WXMSW__
	bool StdHandleOk(DWORD std_handle) {
		// GetStdHandle sometimes returns an invalid handle instead of INVALID_HANDLE_VALUE
		// check with GetHandleInformation
		HANDLE h = GetStdHandle(std_handle);
		DWORD flags;
		return GetHandleInformation(h,&flags);
	}
#endif

void TextIOHandler::init() {
	bool have_stderr;
	#ifdef __WXMSW__
		have_console = false;
		escapes = false;
		// Detect whether to use console output
		have_console = StdHandleOk(STD_OUTPUT_HANDLE);
		have_stderr  = StdHandleOk(STD_ERROR_HANDLE);
		// Detect the --color flag, indicating we should allow escapes
		if (have_console) {
			for (int i = 1 ; i < wxTheApp->argc ; ++i) {
				if (String(wxTheApp->argv[i]) == _("--color")) {
					escapes = true;
					break;
				}
			}
		}
	#else
		// always use console on *nix (?)
		have_console = true;
		have_stderr = true;
		escapes = true; // TODO: detect output redirection
	#endif
	// write to standard output
	stream = stdout;
	raw_mode = false;
	// always write to stderr if possible
	if (have_console) {
		write_errors_to_cli = true;
	}
}

bool TextIOHandler::haveConsole() const {
	return have_console;
}


// ----------------------------------------------------------------------------- : Output

TextIOHandler& TextIOHandler::operator << (const Char* str) {
	if ((escapes && !raw_mode) || str[0] != 27) {
		if (have_console && !raw_mode) {
			IF_UNICODE(fwprintf,fprintf)(stream,str);
		} else {
			buffer += str;
		}
	}
	return *this;
}

TextIOHandler& TextIOHandler::operator << (const String& str) {
	return *this << str.c_str();
}

void TextIOHandler::flush() {
	if (raw_mode) return;
	if (have_console) {
		fflush(stream);
	} else if (!buffer.empty()) {
		// Show message box
		wxMessageBox(buffer, _("Magic Set Editor"), wxOK | wxICON_INFORMATION);
		buffer.clear();
	}
}

// ----------------------------------------------------------------------------- : Input

String TextIOHandler::getLine() {
	String result;
	Char buffer[2048];
	while (!feof(stdin)) {
		if (!IF_UNICODE(fgetws,fgets)(buffer, 2048, stdin)) {
			return result; // error
		}
		result += buffer;
		if (result.GetChar(result.size()-1) == _('\n')) {
			// drop newline, done
			result.resize(result.size() - 1);
			return result;
		}
	}
	return result;
}
bool TextIOHandler::canGetLine() {
	return !feof(stdin);
}

// ----------------------------------------------------------------------------- : Raw mode

void TextIOHandler::enableRaw() {
	raw_mode = true;
	raw_mode_status = 0;
}

void TextIOHandler::flushRaw() {
	if (!raw_mode) return;
	// always end in a newline
	if (!buffer.empty() && buffer.GetChar(buffer.size()-1) != _('\n')) {
		buffer += _('\n');
	}
	// count newlines
	int newline_count = 0;
	FOR_EACH_CONST(c,buffer) if (c==_('\n')) newline_count++;
	// write record
	printf("%d\n%d\n", raw_mode_status, newline_count);
	if (!buffer.empty()) {
		#ifdef UNICODE
			wxCharBuffer buf = buffer.mb_str(wxConvUTF8);
			puts(buf);
		#else
			puts(buffer.c_str());
		#endif
	}
	fflush(stdout);
	// clear
	buffer.clear();
	raw_mode_status = 0;
}

// ----------------------------------------------------------------------------- : Errors

void TextIOHandler::showError(const String& message) {
	stream = stdout;
	*this << RED << _("ERROR: ") << NORMAL << replace_all(message,_("\n"),_("\n       ")) << ENDL;
	flush();
	stream = stdout;
	if (raw_mode) raw_mode_status = max(raw_mode_status, 2);
}

void TextIOHandler::showWarning(const String& message) {
	stream = stdout;
	*this << YELLOW << _("WARNING: ") << NORMAL << replace_all(message,_("\n"),_("\n         ")) << ENDL;
	flush();
	stream = stdout;
	if (raw_mode) raw_mode_status = max(raw_mode_status, 1);
}
