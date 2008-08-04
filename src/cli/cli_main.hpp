//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_CLI_MAIN
#define HEADER_CLI_MAIN

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Command line interface

class CLISetInterface {
  public:
	CLISetInterface();
  private:
	bool quiet;
	bool running;
	
	void run();
	void showWelcome();
	void handleCommand(const String& command);
};

// ----------------------------------------------------------------------------- : EOF
#endif
