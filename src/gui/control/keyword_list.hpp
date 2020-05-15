//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/item_list.hpp>
#include <data/keyword.hpp>
#include <data/filter.hpp>
#include <data/set.hpp>

typedef intrusive_ptr<Filter<Keyword>> KeywordListFilterP;

// ----------------------------------------------------------------------------- : Events

DECLARE_LOCAL_EVENT_TYPE(EVENT_KEYWORD_SELECT, <not used>)
/// Handle KeywordSelectEvents
#define EVT_KEYWORD_SELECT(id, handler) \
  DECLARE_EVENT_TABLE_ENTRY(EVENT_KEYWORD_SELECT, id, -1, \
   (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
   (void (wxEvtHandler::*)(KeywordSelectEvent&)) (&handler), (wxObject*) NULL),

/// The event of selecting a keyword
struct KeywordSelectEvent : public wxCommandEvent {
  KeywordP keyword; ///< The selected keyword
  inline KeywordSelectEvent(const KeywordP& keyword)
    : wxCommandEvent(EVENT_KEYWORD_SELECT), keyword(keyword)
  {}
};

// ----------------------------------------------------------------------------- : KeywordList

/// A control that lists the keywords in a set and its game
class KeywordList : public ItemList, public SetView {
public:
  KeywordList(Window* parent, int id, long additional_style = 0);
  ~KeywordList();
  
  // --------------------------------------------------- : Set stuff
  
  void onBeforeChangeSet() override;
  void onChangeSet() override;
  void onAction(const Action&, bool) override;
  void updateUsageStatistics();
  
  // --------------------------------------------------- : Selection
  
  inline KeywordP getKeyword() const             { return static_pointer_cast<Keyword>(selected_item); }
  inline void     setKeyword(const KeywordP& kw) { selectItem(kw, true, false); }
  
  /// Change the filter to use, can be null
  void setFilter(const KeywordListFilterP& filter);
  
  // --------------------------------------------------- : Clipboard
  
  bool canDelete() const override;
  bool canCopy()   const override;
  bool canPaste()  const override;
  // Try to perform a clipboard operation, return success
  bool doCut() override;
  bool doCopy() override;
  bool doPaste() override;
  bool doDelete() override;
  
  // --------------------------------------------------- : The keywords
protected:
  /// Get a list of all keywords
  void getItems(vector<VoidP>& out) const override;
  /// Return the keyword at the given position in the sorted keyword list
  inline KeywordP getKeyword(long pos) const { return static_pointer_cast<Keyword>(getItem(pos)); }
  
  /// Send an 'item selected' event for the currently selected item (selected_item)
  void sendEvent() override;
  /// Compare keywords
  bool compareItems(void* a, void* b) const override;
  
  /// Get the text of an item in a specific column
  /** Overrides a function from wxListCtrl */
  String OnGetItemText (long pos, long col) const override;
  /// Get the image of an item, by default no image is used
  /** Overrides a function from wxListCtrl */
  int OnGetItemImage(long pos) const override;
  /// Get the color for an item
  wxListItemAttr* OnGetItemAttr(long pos) const override;
  
private:
  void storeColumns();
  
  mutable wxListItemAttr item_attr; // for OnGetItemAttr
  KeywordListFilterP filter; ///< Which keywords to show?
  
  /// How often is a keyword used in the set?
  int usage(const Keyword&) const;
  map<const Keyword*,int> usage_statistics;
  
  // --------------------------------------------------- : Window events
  DECLARE_EVENT_TABLE();
  
  void onContextMenu     (wxContextMenuEvent&);
};

