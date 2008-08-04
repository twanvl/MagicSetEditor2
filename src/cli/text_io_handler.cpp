//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <cli/text_io_handler.hpp>

// ----------------------------------------------------------------------------- : Text I/O handler

const Char* BRIGHT   = _("\x1B[1m");
const Char* NORMAL   = _("\x1B[0m");
const Char* PARAM    = _("\x1B[33m");
const Char* FILE_EXT = _("\x1B[0;1m");
const Char* GRAY     = _("\x1B[1;30m");
const Char* RED      = _("\x1B[1;31m");
const Char* ENDL     = _("\n");

TextIOHandler cli;


void TextIOHandler::init() {
	#ifdef __WXMSW__
		have_console = false;
		escapes = false;
		// Detect whether to use console output
		// GetStdHandle sometimes returns an invalid handle instead of INVALID_HANDLE_VALUE
		// check with GetHandleInformation
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD flags;
		bool ok = GetHandleInformation(h,&flags);
		if (ok) have_console = true;
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
		escapes = true;
	#endif
}

bool TextIOHandler::haveConsole() const {
	return have_console;
}


// ----------------------------------------------------------------------------- : Output

void TextIOHandler::flush() {
	if (have_console) {
		fflush(stdout);
	} else if (!buffer.empty()) {
		// Show message box
		wxMessageBox(buffer, _("Magic Set Editor"), wxOK | wxICON_INFORMATION);
		buffer.clear();
	}
}

TextIOHandler& TextIOHandler::operator << (const Char* str) {
	if (escapes || str[0] != 27) {
		if (have_console) {
			IF_UNICODE(wprintf,printf)(str);
		} else {
			buffer += str;
		}
	}
	return *this;
}

TextIOHandler& TextIOHandler::operator << (const String& str) {
	if (escapes || str.empty() || str.GetChar(0) != 27) {
		if (have_console) {
			IF_UNICODE(wprintf,printf)(str.c_str());
		} else {
			buffer += str;
		}
	}
	return *this;
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
