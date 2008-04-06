//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_KEYWORD_LIST
#define HEADER_GUI_CONTROL_KEYWORD_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/item_list.hpp>
#include <data/keyword.hpp>
#include <data/set.hpp>

// ----------------------------------------------------------------------------- : Events

DECLARE_EVENT_TYPE(EVENT_KEYWORD_SELECT, <not used>)
/// Handle KeywordSelectEvents
#define EVT_KEYWORD_SELECT(id, handler)										\
	DECLARE_EVENT_TABLE_ENTRY(EVENT_KEYWORD_SELECT, id, -1,					\
	 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)		\
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
	
	virtual void onBeforeChangeSet();
	virtual void onChangeSet();
	virtual void onAction(const Action&, bool);
	void updateUsageStatistics();
	
	// --------------------------------------------------- : Selection
	
	inline KeywordP getKeyword() const             { return static_pointer_cast<Keyword>(selected_item); }
	inline void     setKeyword(const KeywordP& kw) { selectItem(kw, true, false); }
	
	// --------------------------------------------------- : Clipboard
	
	bool canCut()   const;
	bool canCopy()  const;
	bool canPaste() const;
	// Try to perform a clipboard operation, return success
	bool doCut();
	bool doCopy();
	bool doPaste();
	
	// --------------------------------------------------- : The keywords
  protected:
	/// Get a list of all keywords
	virtual void getItems(vector<VoidP>& out) const;
	/// Return the keyword at the given position in the sorted keyword list
	inline KeywordP getKeyword(long pos) const { return static_pointer_cast<Keyword>(getItem(pos)); }
	
	/// Send an 'item selected' event for the currently selected item (selected_item)
	virtual void sendEvent();
	/// Compare keywords
	virtual bool compareItems(void* a, void* b) const;
	
	/// Get the text of an item in a specific column
	/** Overrides a function from wxListCtrl */
	virtual String OnGetItemText (long pos, long col) const;
	/// Get the image of an item, by default no image is used
	/** Overrides a function from wxListCtrl */
	virtual int    OnGetItemImage(long pos) const;
	/// Get the color for an item
	virtual wxListItemAttr* OnGetItemAttr(long pos) const;
	
  private:
	void storeColumns();
	
	mutable wxListItemAttr item_attr; // for OnGetItemAttr
	
	/// How often is a keyword used in the set?
	int usage(const Keyword&) const;
	map<const Keyword*,int> usage_statistics;
	
	// --------------------------------------------------- : Window events
	DECLARE_EVENT_TABLE();
	
	void onContextMenu     (wxContextMenuEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
