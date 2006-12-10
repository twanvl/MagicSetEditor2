//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_NEW_WINDOW
#define HEADER_GUI_NEW_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class PackageList;
DECLARE_POINTER_TYPE(Set);

// ----------------------------------------------------------------------------- : NewSetWindow

/// Show the new set window, return the new set, if any
SetP new_set_window(Window* parent);

/// "Create a new set" dialog. First select game, then matching style.
class NewSetWindow : public wxDialog {
  public:
	/// The newly created set, if any
	SetP set;
	
	NewSetWindow(Window* parent);
	
	// --------------------------------------------------- : data
  private:
	DECLARE_EVENT_TABLE();

	// gui items
	PackageList*  game_list, *stylesheet_list;
	wxStaticText* game_text, *stylesheet_text;
	Window* ok_button;
		
	// --------------------------------------------------- : events
	
	void onGameSelect  (wxCommandEvent&);
	
	void onStyleSheetSelect  (wxCommandEvent&);
	void onStyleSheetActivate(wxCommandEvent&);
	
	virtual void OnOK(wxCommandEvent&);
	
	void onUpdateUI(wxUpdateUIEvent&);
	
	// we are done, close the window
	void done();
};

// ----------------------------------------------------------------------------- : EOF
#endif
