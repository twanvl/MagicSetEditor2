//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <data/format/formats.hpp>
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
	wxInitAllImageHandlers();
	initFileFormats();
	settings.read();
	//Window* wnd = new SymbolWindow(nullptr);
	Window* wnd = new SetWindow(nullptr, new_shared1<Set>(Game::byName(_("magic"))));
	wnd->Show();
	return true;
}

// ----------------------------------------------------------------------------- : Exit

int MSE::OnExit() {
	settings.write();
	return 0;
}

// ----------------------------------------------------------------------------- : Exception handling

bool MSE::OnExceptionInMainLoop() {
	return true;
}
