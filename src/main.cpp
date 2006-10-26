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
#include <data/format/formats.hpp>
#include <gui/welcome_window.hpp>
#include <gui/set/window.hpp>
#include <gui/symbol/window.hpp>

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
		wxInitAllImageHandlers();
		init_file_formats();
		settings.read();
		//Window* wnd = new SymbolWindow(nullptr);
		//GameP g = Game::byName(_("magic"))
		SetP  s = new_shared<Set>();
		s->open(_("test.mse-set"));
		Window* wnd = new SetWindow(nullptr, s);
		//Window* wnd = new WelcomeWindow();
		wnd->Show();
		return true;
	
	} catch (Error e) {
		handle_error(e, false);
	} catch (std::exception e) {
		// we don't throw std::exception ourselfs, so this is probably something serious
		handle_error(InternalError(String(e.what(), IF_UNICODE(wxConvLocal, wxSTRING_MAXLEN) )), false);
	} catch (...) {
		handle_error(InternalError(_("An unexpected exception occurred!")), false);
	}
	packages.destroy();
	return false;
}

// ----------------------------------------------------------------------------- : Exit

int MSE::OnExit() {
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
