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
  
  virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
  virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
  virtual void onUpdateUI(wxUpdateUIEvent&);
  virtual void onCommand(int id);
  
  // --------------------------------------------------- : Clipboard
  
  virtual bool canCut()   const;
  virtual bool canCopy()  const;
  virtual bool canPaste() const;
  virtual void doCut();
  virtual void doCopy();
  virtual void doPaste();
  
  protected:
  virtual void onChangeSet();
  
  private:
  SetInfoEditor* editor;
};

