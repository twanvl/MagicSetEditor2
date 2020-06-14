//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/item_list.hpp>
#include <data/card.hpp>
#include <data/set.hpp>

DECLARE_POINTER_TYPE(ChoiceField);
DECLARE_POINTER_TYPE(Field);
class CardListBase;

// ----------------------------------------------------------------------------- : Events

DECLARE_LOCAL_EVENT_TYPE(EVENT_CARD_SELECT,   <not used>)
DECLARE_LOCAL_EVENT_TYPE(EVENT_CARD_ACTIVATE, <not used>)

/// Handle EVENT_CARD_SELECT events
#define EVT_CARD_SELECT(id, handler) \
  DECLARE_EVENT_TABLE_ENTRY(EVENT_CARD_SELECT, id, -1, \
   (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
   (void (wxEvtHandler::*)(CardSelectEvent&)) (&handler), (wxObject*) NULL),

/// Handle EVENT_CARD_ACTIVATE events
#define EVT_CARD_ACTIVATE(id, handler) \
  DECLARE_EVENT_TABLE_ENTRY(EVENT_CARD_ACTIVATE, id, -1, \
   (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
   (void (wxEvtHandler::*)(CardSelectEvent&)) (&handler), (wxObject*) NULL),

/// The event of selecting a card
struct CardSelectEvent : public wxCommandEvent {
  inline CardSelectEvent(int type = EVENT_CARD_SELECT)
    : wxCommandEvent(type)
  {}
  
  /// The selected card
  CardP getCard() const;
  /// All focused cards
  void getSelection(vector<CardP>& out) const;
private:
  CardListBase* getTheCardList() const;
};

// ----------------------------------------------------------------------------- : CardListBase

/// A list view of the cards in a set.
/*  This class allows the cards to be sorted, and has a _('currentCard'), the selected card
 *  when a card is selected, it raises a CardSelectEvent, that will propage to the parent window.
 *
 *  Note: (long) pos refers to position in the sorted_list,
 *        (size_t) index refers to the index in the actual card list.
 */
class CardListBase : public ItemList, public SetView {
public:
  CardListBase(Window* parent, int id, long additional_style = 0);
  ~CardListBase();
  
  // --------------------------------------------------- : Selection
  
  inline CardP getCard() const            { return static_pointer_cast<Card>(selected_item); }
  inline void  setCard(const CardP& card) { selectItem(card, true, false); }
    
  // --------------------------------------------------- : Clipboard
  
  bool canCut()    const override;
  bool canCopy()   const override;
  bool canPaste()  const override;
  bool canDelete() const override;
  // Try to perform a clipboard operation, return success
  bool doCopy() override;
  bool doPaste() override;
  bool doDelete() override;
  
  // --------------------------------------------------- : Set actions
  
  void onBeforeChangeSet() override;
  void onChangeSet() override;
  void onAction(const Action&, bool undone) override;
  
  // --------------------------------------------------- : The cards
public:
  /// Return the card at the given position in the sorted card list
  inline CardP getCard(long pos) const { return static_pointer_cast<Card>(getItem(pos)); }
  /// Get a list of all focused cards
  void getSelection(vector<CardP>& out) const;
protected:
  /// Get a list of all cards
  void getItems(vector<VoidP>& out) const override;
  
  /// Rebuild the card list (clear all vectors and fill them again)
  void rebuild();
  /// Do some additional updating before rebuilding the list
  virtual void onRebuild() {}
  /// Can the card list be modified?
  virtual bool allowModify() const { return false; }
  /// Sort all card lists
  void sortBy(long column, bool ascending) override;
  
  /// Send an 'item selected' event for the currently selected item (selected_item)
  void sendEvent() override { sendEvent(EVENT_CARD_SELECT); }
  void sendEvent(int type = EVENT_CARD_SELECT);
  /// Compare cards
  bool compareItems(void* a, void* b) const override;
  
  // --------------------------------------------------- : Item 'events'
  
  /// Get the text of an item in a specific column
  /** Overrides a function from wxListCtrl */
  String OnGetItemText (long pos, long col) const override;
  /// Get the image of an item, by default no image is used
  /** Overrides a function from wxListCtrl */
  int OnGetItemImage(long pos) const override;
  /// Get the color for an item
  wxListItemAttr* OnGetItemAttr(long pos) const override;
  
  // --------------------------------------------------- : Data
private:
  // display stuff
  vector<FieldP> column_fields; ///< The field to use for each column (by column index)
  FieldP alternate_sort_field;  ///< Second field to sort by, if the column doesn't suffice
  
  mutable wxListItemAttr item_attr; // for OnGetItemAttr
  
public:
  /// Open a dialog for selecting columns to be shown
  void selectColumns();
private:
  /// Store the column sizes in the settings
  void storeColumns();
  
  // --------------------------------------------------- : Window events
  DECLARE_EVENT_TABLE();
  
  void onColumnRightClick(wxListEvent&);
  void onColumnResize    (wxListEvent&);
  void onItemActivate    (wxListEvent&);
  void onSelectColumns   (wxCommandEvent&);
  void onChar            (wxKeyEvent&);
  void onDrag            (wxMouseEvent&);
  void onContextMenu     (wxContextMenuEvent&);
};

