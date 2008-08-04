//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/error.hpp>
#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>

// ----------------------------------------------------------------------------- : Command line interface

CLISetInterface::CLISetInterface()
	: quiet(false)
{
	if (!cli.haveConsole()) {
		throw Error(_("Can not run command line interface without a console;\nstart MSE with \"mse.com --cli\""));
	}
	run();
}

void CLISetInterface::run() {
	// show welcome logo
	if (!quiet) showWelcome();
	// loop
	running = true;
	while (running) {
		// show prompt
		if (!quiet) {
			cli << GRAY << _("> ") << NORMAL;
			cli.flush();
		}
		// read line from stdin
		String command = cli.getLine();
		handleCommand(command);
	}
}

void CLISetInterface::showWelcome() {
	cli << _("                                                                     ___  \n")
	       _("  __  __           _       ___     _      ___    _ _ _              |__ \\ \n")
	       _(" |  \\/  |__ _ __ _(_)__   / __|___| |_   | __|__| (_) |_ ___ _ _       ) |\n")
	       _(" | |\\/| / _` / _` | / _|  \\__ | -_)  _|  | _|/ _` | |  _/ _ \\ '_|     / / \n")
	       _(" |_|  |_\\__,_\\__, |_\\__|  |___|___|\\__|  |___\\__,_|_|\\__\\___/_|      / /_ \n")
	       _("             |___/                                                  |____|\n\n");
	cli.flush();
}

void CLISetInterface::handleCommand(const String& command) {
	if (command.empty()) {
		// empty, ignore
	} else if (command == _(":q") || command == _(":quit")) {
		if (!quiet) {
			cli << _("Goodbye\n"); cli.flush();
		}
		running = false;
	} else if (command == _(":?") || command == _(":help")) {
		// TODO show help
	} else if (command == _("exit") || command == _("quit")) {
		cli << _("Use :quit to quit\n"); cli.flush();
	} else if (command.GetChar(0) == _(':')) {
		cli << _("Unknown command, type :help for help.\n"); cli.flush();
	} else {
		// execute command
		// TODO
		cli << _("You said:\n") << command << ENDL; cli.flush();
	}
}
