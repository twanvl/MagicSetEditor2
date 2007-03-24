//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_KEYWORDS_PANEL
#define HEADER_GUI_SET_KEYWORDS_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class wxSplitterWindow;
class KeywordList;
class TextCtrl;

// ----------------------------------------------------------------------------- : KeywordsPanel

/// A panel for listing and editing the keywords in a set
class KeywordsPanel : public SetWindowPanel {
  public:
	KeywordsPanel(Window* parent, int id);
	
	virtual void onChangeSet();
	
  private:
	// --------------------------------------------------- : Controls
	wxSplitterWindow* splitter;
	KeywordList*      list;
	TextCtrl*         keyword;
	TextCtrl*         match;
	TextCtrl*         reminder;
	TextCtrl*         rules;
};

// ----------------------------------------------------------------------------- : EOF
#endif
