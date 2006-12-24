//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <data/locale.hpp>
#include <data/format/formats.hpp>
#include <gui/welcome_window.hpp>
#include <gui/update_checker.hpp>
#include <gui/set/window.hpp>
#include <gui/symbol/window.hpp>
#include <gui/thumbnail_thread.hpp>
#include <wx/fs_inet.h>

// ----------------------------------------------------------------------------- : Main function/class

/// The application class for MSE.
/** This class is used by wxWidgets as a kind of 'main function'
 */
class MSE : public wxApp {
  public:
	/// Main startup function of the program
	bool OnInit();
	/// On exit: write the settings to the config file
	int  OnExit();
	/// On exception: display error message
	bool OnExceptionInMainLoop();
};

IMPLEMENT_APP(MSE)

// ----------------------------------------------------------------------------- : Initialization

bool MSE::OnInit() {
	try {
		SetAppName(_("Magic Set Editor"));
		wxInitAllImageHandlers();
		wxFileSystem::AddHandler(new wxInternetFSHandler); // needed for update checker
		init_file_formats();
		packages.init();
		settings.read();
		the_locale = Locale::byName(settings.locale);
		check_updates();
		
		// interpret command line
		if (argc > 1) {
			try {
				// Command line argument, find its extension
				wxFileName f(argv[1]);
				if (f.GetExt() == _("mse-symbol")) {
					// Show the symbol editor
					Window* wnd = new SymbolWindow(nullptr, argv[1]);
					wnd->Show();
					return true;
				} else if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
					// Show the set window
					Window* wnd = new SetWindow(nullptr, import_set(argv[1]));
					wnd->Show();
					return true;
				} else {
					handle_error(_("Invalid command line argument:\n") + String(argv[1]));
				}
			} catch (const Error& e) {
				handle_error(e);
			}
		}
		
		// no command line arguments, or error, show welcome window
		(new WelcomeWindow())->Show();
		return true;
			
	} catch (Error e) {
		handle_error(e, false);
	} catch (std::exception e) {
		// we don't throw std::exception ourselfs, so this is probably something serious
		handle_error(InternalError(String(e.what(), IF_UNICODE(wxConvLocal, wxSTRING_MAXLEN) )), false);
	} catch (...) {
		handle_error(InternalError(_("An unexpected exception occurred!")), false);
	}
	OnExit();
	return false;
}

// ----------------------------------------------------------------------------- : Exit

int MSE::OnExit() {
	thumbnail_thread.abortAll();
	settings.write();
	packages.destroy();
	return 0;
}

// ----------------------------------------------------------------------------- : Exception handling

bool MSE::OnExceptionInMainLoop() {
	try {
		throw;	// rethrow the exception, so we can examine it
	} catch (Error e) {
		handle_error(e, false);
	} catch (std::exception e) {
		// we don't throw std::exception ourselfs, so this is probably something serious
		handle_error(InternalError(String(e.what(), IF_UNICODE(wxConvLocal, wxSTRING_MAXLEN) )), false);
	} catch (...) {
		handle_error(InternalError(_("An unexpected exception occurred!")), false);
	}
	return true;
}
