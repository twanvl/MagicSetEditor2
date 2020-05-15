//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

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
  
  void onChangeSet() override;
  void onAction(const Action&, bool undone) override;
  
  // --------------------------------------------------- : UI
  
  void initUI(wxToolBar*, wxMenuBar*) override;
  
  // --------------------------------------------------- : Clipboard
  bool canCut() const override;
  bool canCopy() const override;
  bool canPaste() const override;
  bool canSelectAll() const override;
  void doCut() override;
  void doCopy() override;
  void doPaste() override;
  void doSelectAll() override;
  
  // --------------------------------------------------- : Selection
  void selectCard(const CardP& card) override;
  
private:
  DECLARE_EVENT_TABLE();
  
  CardViewer*    preview;    ///< Card preview
  PackageList*   list;    ///< List of stylesheets
  StylingEditor* editor;    ///< Editor for styling information
  wxButton*      use_for_all;
  wxCheckBox*    use_custom_options;
  CardP          card;    ///< Card we are working on
  
  void onStyleSelect(wxCommandEvent&);
  void onUseForAll(wxCommandEvent&);
  void onUseCustom(wxCommandEvent&);
  
  /// Determine the best size for the list of stylesheets based on available space
  void updateListSize();
  bool Layout() override;
  
  /// Actual intialization of the controls
  void initControls();
};

