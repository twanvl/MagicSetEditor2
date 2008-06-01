//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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
	virtual void onAction(const Action&, bool undone);
	
	// --------------------------------------------------- : Clipboard
	virtual bool canCut() const;
	virtual bool canCopy() const;
	virtual bool canPaste() const;
	virtual void doCut();
	virtual void doCopy();
	virtual void doPaste();
	
	// --------------------------------------------------- : Selection
	virtual void selectCard(const CardP& card);
	
  private:
	DECLARE_EVENT_TABLE();
	
	CardViewer*    preview;		///< Card preview
	PackageList*   list;		///< List of stylesheets
	StylingEditor* editor;		///< Editor for styling information
	wxButton*      use_for_all;
	wxCheckBox*    use_custom_options;
	CardP          card;		///< Card we are working on
	
	void onStyleSelect(wxCommandEvent&);
	void onUseForAll(wxCommandEvent&);
	void onUseCustom(wxCommandEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
