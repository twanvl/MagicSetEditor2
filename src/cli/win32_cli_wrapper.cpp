//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

/** @file cli/win32_cli_wrapper.cpp
 *
 *  Windows is stupid with regards to consoles:
 *    - a program is either 'GUI' or 'console'
 *    - gui programs do not inherit the console of the parent
 *    - hence, there is NO way to printf from a gui program to an existing console
 *
 *  This wrapper hacks around that by opening pipes for std{in,our,err}.
 *  The gui program (MSE) can then use stdout as usual, while this program passes
 *  the text to the real stdout.
 *
 *  In addition, this wrapping allows us to translate a (subset of) ANSI escape codes
 *  so we can use fancy colors.
 *
 */

#ifdef WIN32 // only needed on windows (duh)

// ----------------------------------------------------------------------------- : Includes

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

// ----------------------------------------------------------------------------- : Prototypes

/// How to transfer data from one handle to another
struct Transfer {
	HANDLE from, to;
	bool escapes;
};

DWORD WINAPI TransferThread(Transfer*);
BOOL WINAPI HandleCtrlEvent(DWORD type);
void InitEscapeTranslation(HANDLE console);
void PerformEscapeCode(HANDLE console, char command, int argc, int argv[]);

/// The child process
PROCESS_INFORMATION child_process_info;

/// Pipes / console handles
HANDLE in_mine,  in_theirs,  in_real;
HANDLE out_mine, out_theirs, out_real;
HANDLE err_mine, err_theirs, err_real;

// ----------------------------------------------------------------------------- : Main function

const char* redirect_flags[] = {"-?","--help","-v","--version","--cli","-c","--export","--create-installer"};

int main(int argc, char** argv) {
	// determine whether we need to wrap console i/o
	bool need_redirection = false;
	for (int i = 1 ; i < argc ; ++i) {
		for (int j = 0 ; j < sizeof(redirect_flags)/sizeof(redirect_flags[0]) ; ++j) {
			if (strcmp(argv[i],redirect_flags[j]) == 0) {
				need_redirection = true;
				goto break_2;
			}
		}
	}
	break_2:
	
	// command line
	TCHAR* command_line = GetCommandLine();
	if (need_redirection) {
		// update command line : add --color flag
		TCHAR* my_command_line = command_line;
		command_line = new TCHAR[_tcsclen(command_line) + 10];
		_tcscpy(command_line, my_command_line);
		_tcscat(command_line, _T(" --color"));
	}
	
	// application name
	TCHAR app_path[2048];
	GetModuleFileName(NULL/*current process*/, app_path, sizeof(app_path)/sizeof(TCHAR));
	size_t app_path_length = _tcsclen(app_path);
	if (app_path_length > 4 && _tcsicmp(app_path + app_path_length - 4, _T(".com")) == 0) {
		// replace ".com" with ".exe"
		_tcscpy(app_path + app_path_length - 4, _T(".exe"));
	} else {
		// not a .com file, error message
		fprintf(stderr, "This executable should be named <something>.com\n");
	}
	
	// win32 structures for child program
	STARTUPINFO child_startup_info;
	memset(&child_startup_info, 0, sizeof(child_startup_info));
	memset(&child_process_info, 0, sizeof(child_process_info));
	child_startup_info.cb = sizeof(child_startup_info);
	
	// setup redirection
	if (need_redirection) {
		// Ctrl+C handler
		SetConsoleCtrlHandler(HandleCtrlEvent, TRUE);
		
		// inheritable handles
		SECURITY_ATTRIBUTES inherit;
		inherit.nLength = sizeof(inherit);
		inherit.bInheritHandle = true;
		inherit.lpSecurityDescriptor = NULL;
		
		// create pipes
		CreatePipe(&in_theirs, &in_mine,    &inherit, 0);
		CreatePipe(&out_mine,  &out_theirs, &inherit, 0);
		CreatePipe(&err_mine,  &err_theirs, &inherit, 0);
		
		// the actual handles
		in_real  = GetStdHandle(STD_INPUT_HANDLE);
		out_real = GetStdHandle(STD_OUTPUT_HANDLE);
		err_real = GetStdHandle(STD_ERROR_HANDLE);
		InitEscapeTranslation(out_real);
		
		// start threads
		Transfer tranfer_in  = {in_real,  in_mine,  false};
		Transfer tranfer_out = {out_mine, out_real, true};
		Transfer tranfer_err = {err_mine, err_real, true};
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TransferThread,&tranfer_in, 0,NULL);
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TransferThread,&tranfer_out,0,NULL);
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TransferThread,&tranfer_err,0,NULL);
		
		// give handles to child process
		child_startup_info.hStdInput  = in_theirs;
		child_startup_info.hStdOutput = out_theirs;
		child_startup_info.hStdError  = err_theirs;
		child_startup_info.dwFlags = STARTF_USESTDHANDLES;
	}
	
	// start the child program
	if (!CreateProcess(app_path,command_line,NULL,NULL,TRUE,0,NULL,NULL,&child_startup_info,&child_process_info)) {
		ExitProcess(1);
	}
	
	// wait for program to terminate
	DWORD exit_code = 0;
	if (need_redirection) {
		delete [] command_line;
		WaitForSingleObject(child_process_info.hProcess, INFINITE);
		GetExitCodeProcess(child_process_info.hProcess, &exit_code);
	}
	
	// That's all folks!
	return exit_code;
}

// ----------------------------------------------------------------------------- : Terminating

/// Handle Ctrl+C
BOOL WINAPI HandleCtrlEvent(DWORD type) {
	DWORD exit_code = 1;
	// try to exit child process cleanly
	// TODO: don't exit child on Ctrl+C
	/*CopyFileBuffer(TODO,":quit\n",6);
	if (WaitForSingleObject(child_process_info.hProcess,100) == WAIT_OBJECT_0) {
		GetExitCodeProcess(child_process_info.hProcess, &exit_code);
	} else {*/
		TerminateProcess(child_process_info.hProcess,1);
	//}
	// exit this process
	ExitProcess(exit_code);
	return TRUE;
}

// ----------------------------------------------------------------------------- : I/O redirection

/// Copy a buffer to an output handle
void CopyFileBuffer(HANDLE output, char* buffer, DWORD size) {
	DWORD pos = 0, bytes_written;
	while (pos < size) {
		WriteFile(output, buffer + pos, size - pos, &bytes_written, NULL);
		pos += bytes_written;
	}
}

/// Copy a buffer to an output handle, handling escape code
void CopyFileBufferWithEscape(HANDLE output, char* buffer, DWORD size, bool handle_escapes) {
	if (!handle_escapes) {
		CopyFileBuffer(output, buffer, size);
		return;
	}
	DWORD pos = 0;
	while (pos < size) {
		// find next escape code, "\x1B["
		DWORD next_pos = pos;
		while (next_pos < size &&
			    (buffer[next_pos] != '\x1B' ||
			        (next_pos + 1 >= size || buffer[next_pos+1] != '['))) ++next_pos;
		// copy part before next escape
		CopyFileBuffer(output, buffer+pos, next_pos-pos);
		pos = next_pos;
		// is this an escape code?
		if (pos + 1 < size) {
			// handle escape code
			int argv[10] = {0}, argc = 1;
			for (pos += 2 ; pos < size ; ++pos) {
				if (buffer[pos] == ';') {
					++argc;
				} else if (buffer[pos] >= '0' && buffer[pos] <= '9') {
					argv[argc-1] = 10 * argv[argc-1] + buffer[pos] - '0';
				} else {
					PerformEscapeCode(output, buffer[pos], argc, argv);
					pos++;
					break;
				}
			}
		}
	}
}

/// Thread to transfer text from one handle to another
DWORD WINAPI TransferThread(Transfer* transfer) {
	while (true) {
		// read
		char buffer[1024];
		DWORD bytes_read;
		ReadFile(transfer->from, buffer, sizeof(buffer), &bytes_read, NULL);
		// write
		CopyFileBufferWithEscape(transfer->to, buffer, bytes_read, transfer->escapes);
	}
	return 0;
}

// ----------------------------------------------------------------------------- : Escape codes

#define FOREGROUND_COLOR (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_COLOR (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)

WORD original_attributes;

/// Initialization for escape translation
void InitEscapeTranslation(HANDLE console) {
	CONSOLE_SCREEN_BUFFER_INFO screen_buffer;
	GetConsoleScreenBufferInfo(console, &screen_buffer);
	original_attributes = screen_buffer.wAttributes;
}

/// Perform an escape code
void PerformEscapeCode(HANDLE console, char command, int argc, int argv[]) {
	switch (command) {
		case 'm': {
			CONSOLE_SCREEN_BUFFER_INFO screen_buffer;
			GetConsoleScreenBufferInfo(console, &screen_buffer);
			WORD attributes = screen_buffer.wAttributes;
			for (int i = 0 ; i < argc ; ++i) {
				int major = argv[i] / 10, minor = argv[i] % 10;
				if (argv[i] == 0) { // reset
					attributes = original_attributes;
				} else if (argv[i] == 1) { // bold
					attributes |= FOREGROUND_INTENSITY;
				} else if (argv[i] == 7) { // reverse
					#if BACKGROUND_RED != FOREGROUND_RED << 4
						#error Color codes are not what I expected them to be
					#endif
					attributes = (attributes & (FOREGROUND_COLOR | FOREGROUND_INTENSITY)) << 4
					           | (attributes & (BACKGROUND_COLOR | BACKGROUND_INTENSITY)) >> 4;
				} else if (argv[i] == 21 || argv[i] == 22) { // not bold
					attributes &= ~FOREGROUND_INTENSITY;
				} else if (major == 3) {
					// foreground color
					attributes &= ~FOREGROUND_COLOR;
					if (minor == 9) { // reset
						attributes |= original_attributes & FOREGROUND_COLOR;
					} else {
						attributes |= (minor & 1 ? FOREGROUND_RED       : 0)
						           |  (minor & 2 ? FOREGROUND_GREEN     : 0)
						           |  (minor & 4 ? FOREGROUND_BLUE      : 0);
					}
				} else if (major == 4) {
					// background color
					attributes &= ~BACKGROUND_COLOR;
					if (minor == 9) { // reset
						attributes |= original_attributes & BACKGROUND_COLOR;
					} else {
						attributes |= (minor & 1 ? BACKGROUND_RED       : 0)
						           |  (minor & 2 ? BACKGROUND_GREEN     : 0)
						           |  (minor & 4 ? BACKGROUND_BLUE      : 0);
					}
				} else {
					// other, ignore
				}
			}
			if (attributes != screen_buffer.wAttributes) {
				SetConsoleTextAttribute(console, attributes);
			}
			break;
		}
		default:
			break; // unsupported command, ignore
	}
}

// ----------------------------------------------------------------------------- : EOF
#endif // WIN32
