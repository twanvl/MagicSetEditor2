//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <data/locale.hpp>
#include <data/installer.hpp>
#include <data/format/formats.hpp>
#include <gui/welcome_window.hpp>
#include <gui/update_checker.hpp>
#include <gui/set/window.hpp>
#include <gui/symbol/window.hpp>
#include <gui/thumbnail_thread.hpp>
#include <wx/fs_inet.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

// ----------------------------------------------------------------------------- : Main function/class

/// The application class for MSE.
/** This class is used by wxWidgets as a kind of 'main function'
 */
class MSE : public wxApp {
  public:
	/// Do nothing. The command line parsing, etc. is done in OnRun
	bool OnInit() { return true; }
	/// Main startup function of the program
	/** Use OnRun instead of OnInit, so we can determine whether or not we need a main loop
	 *  Also, OnExit is always run.
	 */
	int OnRun();
	/// On exit: write the settings to the config file
	int  OnExit();
	/// On exception: display error message
	bool OnExceptionInMainLoop();
};

IMPLEMENT_APP(MSE)

// ----------------------------------------------------------------------------- : GUI/Console

/// Write a message to the console if it is available, and to a message box otherwise
void write_stdout(const String& str) {
	bool have_console = false;
	#ifndef __WXMSW__
		// somehow detect whether to use console output
	#endif
	if (have_console) {
		wxFileOutputStream file(wxFile::fd_stdout);
		wxTextOutputStream t(file);
		t.WriteString(str);
	} else {
		// no console, use a message box
		wxMessageBox(str, _("Magic Set Editor"), wxOK | wxICON_INFORMATION);
	}
}

// ----------------------------------------------------------------------------- : Initialization

int MSE::OnRun() {
	try {
		#ifdef __WXMSW__
			SetAppName(_("Magic Set Editor"));
		#else
			// Platform friendly appname
			SetAppName(_("magicseteditor"));
		#endif
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
				String arg = argv[1];
				wxFileName f(argv[1]);
				if (f.GetExt() == _("mse-symbol")) {
					// Show the symbol editor
					Window* wnd = new SymbolWindow(nullptr, argv[1]);
					wnd->Show();
					return wxApp::OnRun();
				} else if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
					// Show the set window
					Window* wnd = new SetWindow(nullptr, import_set(argv[1]));
					wnd->Show();
					return wxApp::OnRun();
				} else if (f.GetExt() == _("mse-installer")) {
					// Installer; install it
					InstallType type = settings.install_type;
					if (argc > 2) {
						String arg = argv[2];
						if (arg.Mid(0,2) == _("--"))
							parse_enum(arg.Mid(2), type);
						}
					Installer::installFrom(argv[1], true, isInstallLocal(type));
					return EXIT_SUCCESS;
				} else if (arg == _("--symbol-editor")) {
					Window* wnd = new SymbolWindow(nullptr);
					wnd->Show();
					return wxApp::OnRun();
				} else if (arg == _("--create-installer")) {
					// create an installer
					Installer inst;
					for (int i = 2 ; i < argc ; ++i) {
						inst.addPackage(argv[i]);
					}
					if (inst.prefered_filename.empty()) {
						throw Error(_("Specify packages to include in installer"));
					} else {
						inst.saveAs(inst.prefered_filename, false);
					}
					return EXIT_SUCCESS;
				} else if (arg == _("--help") || arg == _("-?")) {
					// command line help
					write_stdout( String(_("Magic Set Editor\n\n"))
					            + _("Usage: ") + argv[0] + _("[OPTIONS]\n\n")
					            + _("  no options        \tStart the MSE user interface showing the welcome window.\n")
					            + _("  FILE.mse-set,\n")
					            + _("   FILE.set,\n")
					            + _("   FILE.mse         \tLoad the set file in the MSE user interface.\n")
					            + _("  FILE.mse-symbol   \tLoad the symbol into the MSE symbol editor.\n")
					            + _("  FILE.mse-installer\tInstall the packages from the installer.\n")
					            + _("      --local       \tInstall packages for this user only.\n")
					            + _("  -? --help         \tShows this help screen.\n")
					            + _("  -v --version      \tShow version information.\n")
					            + _("  --symbol-editor   \tShow the symbol editor instead of the welcome window.\n")
					            + _("  --create-installer\n")
					            + _("      FILE [FILE]...\tCreate an instaler named FILE, containing the listed packges.\n") );
					return EXIT_SUCCESS;
				} else if (arg == _("--version") || arg == _("-v")) {
					// dump version
					write_stdout( _("Magic Set Editor\nVersion ") + app_version.toString() + version_suffix );
					return EXIT_SUCCESS;
				} else {
					handle_error(_("Invalid command line argument:\n") + String(argv[1]));
				}
			} catch (const Error& e) {
				handle_error(e);
				return EXIT_FAILURE;
			}
		}
		
		// no command line arguments, or error, show welcome window
		(new WelcomeWindow())->Show();
		return wxApp::OnRun();
		
	} catch (const Error& e) {
		handle_error(e, false);
	} catch (const std::exception& e) {
		// we don't throw std::exception ourselfs, so this is probably something serious
		handle_error(InternalError(String(e.what(), IF_UNICODE(wxConvLocal, wxSTRING_MAXLEN) )), false);
	} catch (...) {
		handle_error(InternalError(_("An unexpected exception occurred!")), false);
	}
	return EXIT_FAILURE;
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
	} catch (const Error& e) {
		handle_error(e, false);
	} catch (const std::exception& e) {
		// we don't throw std::exception ourselfs, so this is probably something serious
		handle_error(InternalError(String(e.what(), IF_UNICODE(wxConvLocal, wxSTRING_MAXLEN) )), false);
	} catch (...) {
		handle_error(InternalError(_("An unexpected exception occurred!")), false);
	}
	return true;
}
