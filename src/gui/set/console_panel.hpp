//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class wxSplitterWindow;
class MessageCtrl;
class HistoryTextCtrl;

// ----------------------------------------------------------------------------- : ConsolePanel

class ConsolePanel : public SetWindowPanel {
public:
  ConsolePanel(Window* parent, int id);
  
  // --------------------------------------------------- : UI
  
  void onIdle(wxIdleEvent&);
  void onEnter(wxCommandEvent&);
  void initUI   (wxToolBar* tb, wxMenuBar* mb) override;
  void destroyUI(wxToolBar* tb, wxMenuBar* mb) override;
  void onUpdateUI(wxUpdateUIEvent&) override;
  void onCommand(int id) override;
  
  // --------------------------------------------------- : Clipboard
  
  bool canCut() const override;
  bool canCopy() const override;
  void doCopy() override;
  
protected:
  void onChangeSet() override;
  void selectCard(const CardP& card) override;
  
private:
  DECLARE_EVENT_TABLE();
  
  CardP card;

  wxSplitterWindow* splitter;
  MessageCtrl* messages;
  wxPanel* entry_panel;
  HistoryTextCtrl* entry;
  
  void get_pending_errors();
  void exec(String const& code);
  
  // notification of new messages
  bool is_active_window;
  MessageType new_errors_since_last_view = MESSAGE_NONE;
  int blinker_state;
  wxTimer blinker_timer;
  static const int MAX_BLINKS = 6;
  static const int BLINK_TIME = 1000;
  
  void stop_blinker();
  void start_blinker();
  void update_blinker();
  void onTimer(wxTimerEvent&);
};

