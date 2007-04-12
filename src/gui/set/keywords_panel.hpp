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
class IconMenu;
struct KeywordSelectEvent;

// ----------------------------------------------------------------------------- : KeywordsPanel

/// A panel for listing and editing the keywords in a set
class KeywordsPanel : public SetWindowPanel {
  public:
	KeywordsPanel(Window* parent, int id);
	~KeywordsPanel();
	
	virtual void onChangeSet();
	
	// --------------------------------------------------- : UI
	
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	
  private:
	DECLARE_EVENT_TABLE();
	
	// --------------------------------------------------- : Controls
	wxSplitterWindow* splitter;
	wxPanel*          panel;
	KeywordList*      list;
	TextCtrl*         keyword;
	TextCtrl*         match;
	TextCtrl*         reminder;
	TextCtrl*         rules;
	IconMenu*         menuKeyword;
	
	// --------------------------------------------------- : Events
	void onKeywordSelect(KeywordSelectEvent& ev);
};

// ----------------------------------------------------------------------------- : EOF
#endif
