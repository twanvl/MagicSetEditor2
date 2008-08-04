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
#include <script/functions/functions.hpp>
#include <data/format/formats.hpp>
#include <wx/process.h>

DECLARE_TYPEOF_COLLECTION(ScriptParseError);

// ----------------------------------------------------------------------------- : Command line interface

CLISetInterface::CLISetInterface(const SetP& set, bool quiet)
	: quiet(quiet)
	, our_context(nullptr)
{
	if (!cli.haveConsole()) {
		throw Error(_("Can not run command line interface without a console;\nstart MSE with \"mse.com --cli\""));
	}
	ei.directory_relative = ei.directory_absolute = wxGetCwd();
	setSet(set);
	run();
}

CLISetInterface::~CLISetInterface() {
	delete our_context;
}

Context& CLISetInterface::getContext() {
	if (set) {
		return set->getContext();
	} else {
		if (!our_context) {
			our_context = new Context();
			init_script_functions(*our_context);
		}
		return *our_context;
	}
}

void CLISetInterface::onBeforeChangeSet() {
	if (set || our_context) {
		Context& ctx = getContext();
		ctx.closeScope(scope);
	}
}

void CLISetInterface::onChangeSet() {
	Context& ctx = getContext();
	scope = ctx.openScope();
	ei.set = set;
}


// ----------------------------------------------------------------------------- : Running

void CLISetInterface::run() {
	// show welcome logo
	if (!quiet) showWelcome();
	// loop
	running = true;
	while (running) {
		if (!cli.canGetLine()) break;
		// show prompt
		if (!quiet) {
			cli << GRAY << _("> ") << NORMAL;
			cli.flush();
		}
		// read line from stdin
		String command = cli.getLine();
		handleCommand(command);
		cli.flush();
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

void CLISetInterface::showUsage() {
	cli << _(" Commands available from the prompt:\n\n");
	cli << _("   <expression>        Execute a script expression, display the result\n");
	cli << _("   :help               Show this help page.\n");
	cli << _("   :load <setfile>     Load a different set file.\n");
	cli << _("   :quit               Exit the MSE command line interface.\n");
	cli << _("   :reset              Clear all local variable definitions.\n");
	cli << _("   :! <command>        Perform a shell command.\n");
	cli << _("\n Commands can be abreviated to their first letter if there is no ambiguity.\n\n");
}

void CLISetInterface::handleCommand(const String& command) {
	try {
		if (command.empty()) {
			// empty, ignore
		} else if (command.GetChar(0) == _(':')) {
			// :something
			size_t space = min(command.find_first_of(_(' ')), command.size());
			String before = command.substr(0,space);
			String arg    = space + 1 < command.size() ? command.substr(space+1) : wxEmptyString;
			if (before == _(":q") || before == _(":quit")) {
				if (!quiet) {
					cli << _("Goodbye\n");
				}
				running = false;
			} else if (before == _(":?") || before == _(":h") || before == _(":help")) {
				showUsage();
			} else if (before == _(":l") || before == _(":load")) {
				if (arg.empty()) {
					cli << _("Give a filename to open.\n");
				} else {
					setSet(import_set(arg));
				}
			} else if (before == _(":r") || before == _(":reset")) {
				Context& ctx = getContext();
				ctx.closeScope(scope);
				scope = ctx.openScope();
			} else if (before == _(":!")) {
				if (arg.empty()) {
					cli << _("Give a shell command to execute.\n");
				} else {
					#ifdef __WXMSW__
						_wsystem(arg.c_str());
					#elif UNICODE
						wxCharBuffer buf = arg.fn_str();
						system(buf);
					#else
						system(arg.c_str());
					#endif
				}
			} else {
				cli << _("Unknown command, type :help for help.\n");
			}
		} else if (command == _("exit") || command == _("quit")) {
			cli << _("Use :quit to quit\n");
		} else if (command == _("help")) {
			cli << _("Use :help for help\n");
		} else {
			// parse command
			vector<ScriptParseError> errors;
			ScriptP script = parse(command,nullptr,false,errors);
			if (!errors.empty()) {
				FOR_EACH(error,errors) showError(error.what());
				return;
			}
			// execute command
			WITH_DYNAMIC_ARG(export_info, &ei);
			Context& ctx = getContext();
			ScriptValueP result = ctx.eval(*script,false);
			// show result
			cli << result->toCode() << ENDL;
		}
	} catch (const Error& e) {
		showError(e.what());
	}
}

void CLISetInterface::showError(const String& error) {
	cli << RED << _("ERROR: ") << NORMAL << replace_all(error,_("\n"),_("\n       ")) << ENDL;
}
