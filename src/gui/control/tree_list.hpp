//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_TREE_LIST
#define HEADER_GUI_CONTROL_TREE_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/vscroll.h>

// ----------------------------------------------------------------------------- : TreeList

/// A combination of a TreeCtrl and a ListCtrl. A tree with multiple columns.
class TreeList : public wxPanel {
  public:
	TreeList(Window* parent, int id, long style = wxSUNKEN_BORDER);
	
	/// Expand/collapse an item
	void expand(size_t item, bool expand = true);
	/// Select an item
	void select(size_t item, bool event = true);
	
	/// (re)build the list
	void rebuild(bool full = true);
	
  public:
	
	/// An item in the tree list
	class Item : public IntrusivePtrBase<Item> {
	  public:
		Item() : level(0), expanded(false) {}
		virtual ~Item() {}
		
		int    level;
		bool   expanded;
		inline bool visible() const { return position != NOTHING; }
		
	  private:
		friend class TreeList;
		size_t position; // NOTHING if invisible, otherwise the line the item is on
		UInt   lines;    // lines in front of this item (bit set)
	};
	typedef intrusive_ptr<Item> ItemP;
	
  protected:
	
	/// The items in the tree list
	vector<ItemP> items;
	
	static const size_t NOTHING = (size_t)-1;
	size_t selection;
	
	/// Initialize the items
	virtual void initItems() = 0;
	
	/// Draw the text of an item
	virtual void drawItem(DC& dc, size_t index, size_t column, int x, int y, bool selected) const = 0;
	
	/// The number of columns
	virtual size_t columnCount() const = 0;
	/// The text of a column
	virtual String columnText(size_t column) const = 0;
	/// The width of a column in pixels
	virtual int    columnWidth(size_t column) const = 0;
	
	int    item_height;
	static const int header_height = 17;
	static const int level_width   = 17;
	
  private:
	size_t total_lines;     // number of shown items
	size_t first_line;      // first visible line
	size_t visible_lines;   // number of (partially) visible lines
	size_t visible_lines_t; // number of totally visible lines
	
	void calcItemCount();
	size_t findItemByPos(int y) const;
	size_t findItem(size_t line, size_t start = 0) const;
	size_t findLastItem(size_t end) const;
	size_t findParent(size_t item) const;
	bool hasChildren(size_t item) const;
	
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
	void onChar(wxKeyEvent& ev);
	void onLeftDown(wxMouseEvent& ev);
	void onLeftDClick(wxMouseEvent& ev);
	
	// VScrolledWindow clone
	
	void onSize(wxSizeEvent& ev);
	void onScroll(wxScrollWinEvent& ev);
	void onMouseWheel(wxMouseEvent& ev);
	void ScrollToLine(size_t line);
	void UpdateScrollbar();
	void RefreshLine(size_t line);
};

// ----------------------------------------------------------------------------- : EOF
#endif
