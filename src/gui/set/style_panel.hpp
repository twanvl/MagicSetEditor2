//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_STYLE_PANEL
#define HEADER_GUI_SET_STYLE_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class CardViewer;
class PackageList;
class StylingEditor;

// ----------------------------------------------------------------------------- : StylePanel

/// A panel showing a list of stylesheets, and an editor for styling
class StylePanel : public SetWindowPanel {
  public:
	StylePanel(Window* parent, int id);
	
	virtual void onChangeSet();
	
	// --------------------------------------------------- : Selection
	virtual void selectCard(const CardP& card);
	
  private:
	CardViewer*    preview;		///< Card preview
	PackageList*   list;		///< List of stylesheets
	StylingEditor* editor;		///< Editor for styling information
	wxButton*      use_for_all;
};

// ----------------------------------------------------------------------------- : EOF
#endif
