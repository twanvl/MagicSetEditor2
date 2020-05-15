//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class SetInfoEditor;

// ----------------------------------------------------------------------------- : SetInfoPanel

class SetInfoPanel : public SetWindowPanel {
public:
  SetInfoPanel(Window* parent, int id);
  
  // --------------------------------------------------- : UI
  
  void initUI   (wxToolBar* tb, wxMenuBar* mb) override;
  void destroyUI(wxToolBar* tb, wxMenuBar* mb) override;
  void onUpdateUI(wxUpdateUIEvent&) override;
  void onCommand(int id) override;
  
  // --------------------------------------------------- : Clipboard
  
  bool canCut() const override;
  bool canCopy() const override;
  bool canPaste() const override;
  bool canSelectAll() const override;
  void doCut() override;
  void doCopy() override;
  void doPaste() override;
  void doSelectAll() override;
  
protected:
  void onChangeSet() override;
  
private:
  SetInfoEditor* editor;
};

