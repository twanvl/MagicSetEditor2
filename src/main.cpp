//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <util/spell_checker.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <data/locale.hpp>
#include <data/installer.hpp>
#include <data/format/formats.hpp>
#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>
#include <gui/welcome_window.hpp>
#include <gui/update_checker.hpp>
#include <gui/packages_window.hpp>
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
	/// Actually start the GUI mainloop
	int runGUI();
	/// On exit: write the settings to the config file
	int OnExit();
	/// On exception: display error message
	bool OnExceptionInMainLoop();
	/// Fancier assert
	#if defined(_MSC_VER) && defined(_DEBUG)
		void OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg);
	#endif
};

IMPLEMENT_APP(MSE)

// ----------------------------------------------------------------------------- : Checks

void nag_about_ascii_version() {
	#if !defined(UNICODE) && defined(__WXMSW__)
		// windows 2000/XP/Vista/... users shouldn't use the 9x version
		OSVERSIONINFO info;
		info.dwOSVersionInfoSize = sizeof(info);
		GetVersionEx(&info);
		if (info.dwMajorVersion >= 5) {
			handle_warning(_("This build of Magic Set Editor is intended for Windows 95/98/ME systems.\n")
			               _("It is recommended that you download the appropriate MSE version for your Windows version."));
		}
	#endif
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
		init_script_variables();
		init_file_formats();
		cli.init();
		package_manager.init();
		settings.read();
		the_locale = Locale::byName(settings.locale);
		nag_about_ascii_version();
		
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
					return runGUI();
				} else if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
					// Show the set window
					Window* wnd = new SetWindow(nullptr, import_set(argv[1]));
					wnd->Show();
					return runGUI();
				} else if (f.GetExt() == _("mse-installer")) {
					// Installer; install it
					InstallType type = settings.install_type;
					if (argc > 2) {
						String arg = argv[2];
						if (starts_with(argv[2], _("--")) && arg != _("--color")) {
							parse_enum(String(argv[2]).substr(2), type);
						}
					}
					InstallerP installer = open_package<Installer>(argv[1]);
					PackagesWindow wnd(nullptr, installer);
					wnd.ShowModal();
					return EXIT_SUCCESS;
				} else if (arg == _("--symbol-editor")) {
					Window* wnd = new SymbolWindow(nullptr);
					wnd->Show();
					return runGUI();
				} else if (arg == _("--create-installer")) {
					// create an installer
					Installer inst;
					for (int i = 2 ; i < argc ; ++i) {
						if (!starts_with(argv[i],_("--"))) {
							inst.addPackage(argv[i]);
						}
					}
					if (inst.prefered_filename.empty()) {
						throw Error(_("Specify packages to include in installer"));
					} else {
						inst.saveAs(inst.prefered_filename, false);
					}
					return EXIT_SUCCESS;
				} else if (arg == _("--help") || arg == _("-?")) {
					// command line help
					cli << _("Magic Set Editor\n\n");
					cli << _("Usage: ") << BRIGHT << argv[0] << NORMAL << _(" [") << PARAM << _("OPTIONS") << NORMAL << _("]");
					cli << _("\n\n  no options");
					cli << _("\n         \tStart the MSE user interface showing the welcome window.");
					cli << _("\n\n  ") << BRIGHT << _("-?") << NORMAL << _(", ")
					                   << BRIGHT << _("--help") << NORMAL;
					cli << _("\n         \tShows this help screen.");
					cli << _("\n\n  ") << BRIGHT << _("-v") << NORMAL << _(", ")
					                   << BRIGHT << _("--version") << NORMAL;
					cli << _("\n         \tShow version information.");
					cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-set") << NORMAL << _(", ")
					                   << PARAM << _("FILE") << FILE_EXT << _(".set") << NORMAL << _(", ")
					                   << PARAM << _("FILE") << FILE_EXT << _(".mse") << NORMAL;
					cli << _("\n         \tLoad the set file in the MSE user interface.");
					cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-symbol") << NORMAL;
					cli << _("\n         \tLoad the symbol into the MSE symbol editor.");
					cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-installer")
					                   << NORMAL << _(" [") << BRIGHT << _("--local") << NORMAL << _("]");
					cli << _("\n         \tInstall the packages from the installer.");
					cli << _("\n         \tIf the ") << BRIGHT << _("--local") << NORMAL << _(" flag is passed, install packages for this user only.");
					cli << _("\n\n  ") << BRIGHT << _("--symbol-editor") << NORMAL;
					cli << _("\n         \tShow the symbol editor instead of the welcome window.");
					cli << _("\n\n  ") << BRIGHT << _("--create-installer") << NORMAL << _(" [")
					                   << PARAM << _("OUTFILE") << FILE_EXT << _(".mse-installer") << NORMAL << _("] [")
					                   << PARAM << _("PACKAGE") << NORMAL << _(" [") << PARAM << _("PACKAGE") << NORMAL << _(" ...]]");
					cli << _("\n         \tCreate an instaler, containing the listed packages.");
					cli << _("\n         \tIf no output filename is specified, the name of the first package is used.");
					cli << _("\n\n  ") << BRIGHT << _("--export") << NORMAL << PARAM << _(" FILE") << NORMAL << _(" [") << PARAM << _("IMAGE") << NORMAL << _("]");
					cli << _("\n         \tExport the cards in a set to image files,");
					cli << _("\n         \tIMAGE is the same format as for 'export all card images'.");
					cli << _("\n\n  ") << BRIGHT << _("--cli") << NORMAL << _(" [")
					                   << PARAM << _("FILE") << NORMAL << _("] [")
					                   << BRIGHT << _("--quiet") << NORMAL << _("] [")
					                   << BRIGHT << _("--raw") << NORMAL << _("]");
					cli << _("\n         \tStart the command line interface for performing commands on the set file.");
					cli << _("\n         \tUse ") << BRIGHT << _("-q") << NORMAL << _(" or ") << BRIGHT << _("--quiet") << NORMAL << _(" to supress the startup banner and prompts.");
					cli << _("\n         \tUse ") << BRIGHT << _("-raw") << NORMAL << _(" for raw output mode.");
					cli << _("\n\nRaw output mode is intended for use by other programs:");
					cli << _("\n    - The only output is only in response to commands.");
					cli << _("\n    - For each command a single 'record' is written to the standard output.");
					cli << _("\n    - The record consists of:");
					cli << _("\n        - A line with an integer status code, 0 for ok, 1 for warnings, 2 for errors");
					cli << _("\n        - A line containing an integer k, the number of lines to follow");
					cli << _("\n        - k lines, each containing UTF-8 encoded string data.");
					cli << ENDL;
					cli.flush();
					return EXIT_SUCCESS;
				} else if (arg == _("--version") || arg == _("-v")) {
					// dump version
					cli << _("Magic Set Editor\n");
					cli << _("Version ") << app_version.toString() << version_suffix << ENDL;
					cli.flush();
					return EXIT_SUCCESS;
				} else if (arg == _("--cli")) {
					// command line interface
					SetP set;
					bool quiet = false;
					for (int i = 2 ; i < argc ; ++i) {
						String arg = argv[i];
						wxFileName f(argv[i]);
						if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
							set = import_set(arg);
						} else if (arg == _("-q") || arg == _("--quiet")) {
							quiet = true;
						} else if (arg == _("-r") || arg == _("--raw")) {
							quiet = true;
							cli.enableRaw();
						}
					}
					CLISetInterface cli_interface(set,quiet);
					return EXIT_SUCCESS;
				} else if (arg == _("--export")) {
					if (argc <= 2 || argc <= 3 && starts_with(argv[2],_("--"))) {
						handle_error(Error(_("No input file specified for --export")));
						return EXIT_FAILURE;
					}
					SetP set = import_set(argv[2]);
					// path
					String out = argc >= 3 && !starts_with(argv[3],_("--"))
					           ? argv[3]
					           : settings.gameSettingsFor(*set->game).images_export_filename;
					String path = _(".");
					size_t pos = out.find_last_of(_("/\\"));
					if (pos != String::npos) {
						path = out.substr(0, pos);
						if (!wxDirExists(path)) wxMkdir(path);
						path += _("/x");
						out  = out.substr(pos + 1);
					}
					// export
					export_images(set, set->cards, path, out, CONFLICT_NUMBER_OVERWRITE);
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
		return runGUI();
		
	} CATCH_ALL_ERRORS(true);
	return EXIT_FAILURE;
}

int MSE::runGUI() {
	check_updates();
	return wxApp::OnRun();
}

// ----------------------------------------------------------------------------- : Exit

int MSE::OnExit() {
	thumbnail_thread.abortAll();
	settings.write();
	package_manager.destroy();
	SpellChecker::destroyAll();
	return 0;
}

// ----------------------------------------------------------------------------- : Exception handling

bool MSE::OnExceptionInMainLoop() {
	try {
		throw;	// rethrow the exception, so we can examine it
	} CATCH_ALL_ERRORS(true);
	return true;
}

#if defined(_MSC_VER) && defined(_DEBUG)
	// Print assert failures to debug output
	void msvc_assert(const char*, const char*, const char*, unsigned);
	void MSE::OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg) {
		#ifdef UNICODE
			char file_[1024]; wcstombs(file_,file,1023);
			char cond_[1024]; wcstombs(cond_,cond,1023);
			char msg_ [1024]; wcstombs(msg_, msg, 1023);
			msvc_assert(msg_, cond_, file_, line);
		#else
			msvc_assert(msg, cond, file, line);
		#endif
	}
#endif
