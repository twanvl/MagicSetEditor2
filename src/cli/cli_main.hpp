//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_CLI_MAIN
#define HEADER_CLI_MAIN

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/set.hpp>

// ----------------------------------------------------------------------------- : Command line interface

class CLISetInterface : public SetView {
  public:
	CLISetInterface();
	~CLISetInterface();
  protected:
	void onAction(const Action&, bool) {}
  private:
	bool quiet;    ///< Supress prompts and other non-vital stuff
	bool running;  ///< Still running?
	
	void run();
	void showWelcome();
	void showUsage();
	void handleCommand(const String& command);
	void showError(const String& error);
	
	/// our own context, when no set is loaded
	Context& getContext();
	Context* our_context;
};

// ----------------------------------------------------------------------------- : EOF
#endif
