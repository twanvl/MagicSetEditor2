//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>
#include <data/pack.hpp>
#include <wx/spinctrl.h>

class CardViewer;
class RandomCardList;
class PackTotalsPanel;
class SelectableLabel;
struct CardSelectEvent;
DECLARE_POINTER_TYPE(PackType);

// ----------------------------------------------------------------------------- : Utility

// for lists of spin controls
struct PackAmountPicker {
  PackTypeP        pack;
  SelectableLabel* label;
  wxSpinCtrl*      value;
  
  PackAmountPicker() {}
  PackAmountPicker(wxWindow* parent, wxFlexGridSizer* sizer, const PackTypeP& pack, bool interactive);
  void destroy(wxFlexGridSizer* sizer);
};

// ----------------------------------------------------------------------------- : RandomPackPanel

/// A SetWindowPanel for creating random booster packs
class RandomPackPanel : public SetWindowPanel {
public:
  RandomPackPanel(Window* parent, int id);
  ~RandomPackPanel();
  
  // --------------------------------------------------- : UI
  
  void onBeforeChangeSet() override;
  void onChangeSet() override;
  void onAction(const Action&, bool undone) override;
  
  void initUI   (wxToolBar* tb, wxMenuBar* mb) override;
  void destroyUI(wxToolBar* tb, wxMenuBar* mb) override;
  void onUpdateUI(wxUpdateUIEvent&) override;
  void onCommand(int id) override;
  
  // --------------------------------------------------- : Selection
  CardP selectedCard() const override;
  void selectCard(const CardP& card) override;
  void selectionChoices(ExportCardSelectionChoices& out) override;
  
  // --------------------------------------------------- : Clipboard
  
  bool canCopy() const override;
  void doCopy() override;
  
private:
  DECLARE_EVENT_TABLE();
  
  CardViewer*       preview;    ///< Card preview
  RandomCardList*   card_list;  ///< The list of cards
  wxTextCtrl*       seed;      ///< Seed value
  wxFlexGridSizer*  packsSizer;
  wxFlexGridSizer*  totalsSizer;
  wxButton*         generate_button;
  wxRadioButton*    seed_random, *seed_fixed;
  PackTotalsPanel*  totals;
  vector<PackAmountPicker> pickers;
  
  PackGenerator generator;
  int last_seed;
  
  /// Actual intialization of the controls
  void initControls();
  
  /// Update the total count of each card type
  void updateTotals();
  /// Get a seed value
  int getSeed();
  void setSeed(int seed);
  /// Generate the cards
  void generate();
  /// Store the settings
  void storeSettings();
  
  void onCardSelect(CardSelectEvent& ev);
  void onPackTypeClick(wxCommandEvent& ev);
public:
  typedef PackItem PackItem_for_typeof;
};

