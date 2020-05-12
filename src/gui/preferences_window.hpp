//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Preferences window

/// Dialog for the program settings, rendered as a set of pages
class PreferencesWindow : public wxDialog {
public:
  PreferencesWindow(Window* parent);
  
private:
  DECLARE_EVENT_TABLE();
  
  /// Close the dialog, and store all settings
  void onOk(wxCommandEvent&);
};

