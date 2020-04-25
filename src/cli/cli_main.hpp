//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/set.hpp>
#include <data/export_template.hpp>
#include <script/profiler.hpp>

// ----------------------------------------------------------------------------- : Command line interface

class CLISetInterface : public SetView {
public:
  /// The set is optional
  CLISetInterface(const SetP& set, bool quiet = false, bool run = true);
protected:
  virtual void onAction(const Action&, bool) {}
  virtual void onChangeSet();
  virtual void onBeforeChangeSet();
private:
  bool quiet;    ///< Supress prompts and other non-vital stuff
  bool running;  ///< Still running?
  
  void run();
  void showWelcome();
  void showUsage();
  void handleCommand(const String& command);
  #if USE_SCRIPT_PROFILING
    void showProfilingStats(const FunctionProfile& parent, int level = 0);
  #endif
  
  /// our own context, when no set is loaded
  Context& getContext();
  unique_ptr<Context> our_context;
  size_t scope;
  
  // export info, so we can write files
  ExportInfo ei;
  void setExportInfoCwd();
};

bool run_script_file(String const& filename);

