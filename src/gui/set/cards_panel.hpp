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
class FilteredImageCardList;
class DataEditor;
class TextCtrl;
class HoverButton;
class FindInfo;
class FilterCtrl;

// ----------------------------------------------------------------------------- : CardsPanel

/// A card list and card editor panel
class CardsPanel : public SetWindowPanel {
public:
  CardsPanel(Window* parent, int id);
  ~CardsPanel();
  
  void onChangeSet() override;
  
  // --------------------------------------------------- : UI
  
  void initUI   (wxToolBar* tb, wxMenuBar* mb) override;
  void destroyUI(wxToolBar* tb, wxMenuBar* mb) override;
  void onUpdateUI(wxUpdateUIEvent&) override;
  void onCommand(int id) override;
  void onMenuOpen(wxMenuEvent&) override;
  
  // --------------------------------------------------- : Actions
  
  bool wantsToHandle(const Action&, bool undone) const override;
  
  // --------------------------------------------------- : Clipboard
  bool canCut() const override;
  bool canCopy() const override;
  bool canPaste() const override;
  void doCut() override;
  void doCopy() override;
  void doPaste() override;
  
  // --------------------------------------------------- : Text selection

  bool canSelectAll() const override;
  void doSelectAll() override;

  // --------------------------------------------------- : Searching (find/replace)

  bool canFind()    const override { return true; }
  bool canReplace() const override { return true; }
  bool doFind      (wxFindReplaceData&) override;
  bool doReplace   (wxFindReplaceData&) override;
  bool doReplaceAll(wxFindReplaceData&) override;
private:
  /// Do a search or replace action for the given FindInfo in all cards
  bool search(FindInfo& find, bool from_start);
  class SearchFindInfo;
  class ReplaceFindInfo;
  friend class CardsPanel::SearchFindInfo;
  friend class CardsPanel::ReplaceFindInfo;
public:

  // --------------------------------------------------- : Selection
  CardP selectedCard() const override;
  void selectCard(const CardP& card) override;
  void selectFirstCard() override;

private:
  // --------------------------------------------------- : Controls
  wxSizer*          s_left;
  wxSplitterWindow* splitter;
  DataEditor*       editor;
  FilteredImageCardList* card_list;
  wxPanel*          nodes_panel;
  TextCtrl*         notes;
  HoverButton*      collapse_notes;
  FilterCtrl*       filter;
  String            filter_value; // value of filter, need separate variable because the control is destroyed
  bool              notes_below_editor;
  
  /// Move the notes panel below the editor or below the card list
  void updateNotesPosition();
  // before Layout, call updateNotesPosition.
  // NOTE: docs say this function returns void, but the code says bool
  bool Layout() override;
  
  // --------------------------------------------------- : Menus & tools
  wxMenu* menuCard, *menuFormat;
  wxToolBarToolBase* toolAddCard;
  wxMenuItem* insertSymbolMenu;    // owned by menuFormat, but submenu owned by SymbolFont
  wxMenuItem* insertManyCardsMenu; // owned my menuCard, but submenu can be changed
  
  wxMenu* makeAddCardsSubmenu(bool add_single_card_option);
};

