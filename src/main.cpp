//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
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
#include <wx/socket.h>

ScriptValueP export_set(SetP const& set, vector<CardP> const& cards, ExportTemplateP const& exp, String const& outname);

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
  void HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const;
  /// Hack around some wxWidget idiocies
  int FilterEvent(wxEvent& ev);
  /// Fancier assert
  #if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
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
      queue_message(MESSAGE_WARNING,
        _("This build of Magic Set Editor is intended for Windows 95/98/ME systems.\n")
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
    wxSocketBase::Initialize();
    init_script_variables();
    init_file_formats();
    cli.init();
    package_manager.init();
    settings.read();
    the_locale = Locale::byName(settings.locale);
    nag_about_ascii_version();
    
    // interpret command line
    {
      // ingnore the --color argument, it is handled by cli.init()
      vector<String> args;
      for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
        if (args.back() == _("--color")) args.pop_back();
      }
      if (!args.empty()) {
        const String& arg = args[0];
        // Find the extension
        wxFileName f(arg.Mid(0,arg.find_last_not_of(_("\\/")) + 1));
        if (f.GetExt() == _("mse-symbol")) {
          // Show the symbol editor
          Window* wnd = new SymbolWindow(nullptr, arg);
          wnd->Show();
          return runGUI();
        } else if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
          // Show the set window
          Window* wnd = new SetWindow(nullptr, import_set(arg));
          wnd->Show();
          return runGUI();
        } else if (f.GetExt() == _("mse-installer")) {
          // Installer; install it
          InstallType type = settings.install_type;
          if (args.size() > 1) {
            if (starts_with(args[1], _("--"))) {
              parse_enum(String(args[1]).substr(2), type);
            }
          }
          InstallerP installer = open_package<Installer>(argv[1]);
          PackagesWindow wnd(nullptr, installer);
          wnd.ShowModal();
          return EXIT_SUCCESS;
        } else if (f.GetExt() == _("mse-script")) {
          // Run a script file
          if (!run_script_file(arg)) return EXIT_FAILURE;
          if (cli.shown_errors()) return EXIT_FAILURE;
          return EXIT_SUCCESS;
        } else if (arg == _("--symbol-editor")) {
          Window* wnd = new SymbolWindow(nullptr);
          wnd->Show();
          return runGUI();
        } else if (arg == _("--create-installer")) {
          // create an installer
          Installer inst;
          FOR_EACH(arg, args) {
            if (!starts_with(arg, _("--"))) {
              inst.addPackage(arg);
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
          cli << _("\n\n  ") << BRIGHT << _("--export") << NORMAL << PARAM << _(" TEMPLATE SETFILE ") << NORMAL << _(" [") << PARAM << _("OUTFILE") << NORMAL << _("]");
          cli << _("\n         \tExport a set using an export template.");
          cli << _("\n         \tIf no output filename is specified, the result is written to stdout.");
          cli << _("\n\n  ") << BRIGHT << _("--export-images") << NORMAL << PARAM << _(" FILE") << NORMAL << _(" [") << PARAM << _("IMAGE") << NORMAL << _("]");
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
        } else if (arg == _("--version") || arg == _("-v") || arg == _("-V")) {
          // dump version
          cli << _("Magic Set Editor\n");
          cli << _("Version ") << app_version.toString() << version_suffix << ENDL;
          cli.flush();
          return EXIT_SUCCESS;
        } else if (arg == _("--cli")) {
          // command line interface
          SetP set;
          bool quiet = false;
          for (size_t i = 1; i < args.size(); ++i) {
            String const& arg = args[i];
            wxFileName f(arg);
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
        } else if (arg == _("--export-images")) {
          if (args.size() < 2) {
            handle_error(Error(_("No input file specified for --export")));
            return EXIT_FAILURE;
          }
          SetP set = import_set(args[1]);
          // path
          String out = args.size() >= 3 && !starts_with(args[2], _("--"))
            ? args[2]
            : settings.gameSettingsFor(*set->game).images_export_filename;
          String path = _(".");
          size_t pos = out.find_last_of(_("/\\"));
          if (pos != String::npos) {
            path = out.substr(0, pos);
            if (!wxDirExists(path)) wxMkdir(path);
            path += _("/x");
            out = out.substr(pos + 1);
          }
          // export
          export_images(set, set->cards, path, out, CONFLICT_NUMBER_OVERWRITE);
          return EXIT_SUCCESS;
        } else if (args[0] == _("--export")) {
          if (args.size() < 2) {
            throw Error(_("No export template specified for --export"));
          } else if (args.size() < 3) {
            throw Error(_("No input set file specified for --export"));
          }
          String export_template = args[1];
          ExportTemplateP exp = ExportTemplate::byName(export_template);
          SetP set = import_set(args[2]);
          String out = args.size() >= 4 ? args[3] : _("");
          ScriptValueP result = export_set(set, set->cards, exp, out);
          if (out.empty()) {
            cli << result->toString();
          }
          return EXIT_SUCCESS;
        } else {
          handle_error(_("Invalid command line argument:\n") + arg);
        }
      }
    }
    
    // no command line arguments, or error, show welcome window
    (new WelcomeWindow())->Show();
    return runGUI();
    
  } CATCH_ALL_ERRORS(true);
  cli.print_pending_errors();
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

void MSE::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const {
  try {
    wxApp::HandleEvent(handler, func, event);
  } CATCH_ALL_ERRORS(true);
}

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
  // Print assert failures to debug output
  void MSE::OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg) {
    #ifdef UNICODE
      msvc_assert(msg, cond, file, line);
    #else
      wchar_t file_[1024]; mbstowcs(file_,file,1023);
      wchar_t cond_[1024]; mbstowcs(cond_,cond,1023);
      wchar_t msg_ [1024]; mbstowcs(msg_, msg, 1023);
      msvc_assert(msg_, cond_, file_, line);
    #endif
  }
#endif

// ----------------------------------------------------------------------------- : Events

int MSE::FilterEvent(wxEvent& ev) {
  /*if (ev.GetEventType() == wxEVT_MOUSE_CAPTURE_LOST) {
    return 1;
  } else {
    return -1;
  }*/
  return -1;
}
