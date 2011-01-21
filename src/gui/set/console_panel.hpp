//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_CONSOLE_PANEL
#define HEADER_GUI_SET_CONSOLE_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class wxSplitterWindow;
class MessageCtrl;
class TextCtrl;

// ----------------------------------------------------------------------------- : ConsolePanel

class ConsolePanel : public SetWindowPanel {
  public:
	ConsolePanel(Window* parent, int id);
	
	// --------------------------------------------------- : UI
	
	void onIdle(wxIdleEvent&);
	void onEnter(wxCommandEvent&);
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	
	// --------------------------------------------------- : Clipboard
	
	/*
	virtual bool canCut()   const;
	virtual bool canCopy()  const;
	virtual bool canPaste() const;
	virtual void doCut();
	virtual void doCopy();
	virtual void doPaste();
	*/
	
  protected:
	virtual void onChangeSet();
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxSplitterWindow* splitter;
	MessageCtrl* messages;
	wxPanel* entry_panel;
	wxTextCtrl* entry;
	
	void get_pending_errors();
	void exec(String const& code);
};

// ----------------------------------------------------------------------------- : EOF
#endif
