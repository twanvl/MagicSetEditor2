//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_GALLERY_LIST
#define HEADER_GUI_CONTROL_GALLERY_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>

// ----------------------------------------------------------------------------- : Events

DECLARE_EVENT_TYPE(EVENT_GALLERY_SELECT,   <not used>)
DECLARE_EVENT_TYPE(EVENT_GALLERY_ACTIVATE, <not used>)

/// Handle EVENT_GALLERY_SELECT events
#define EVT_GALLERY_SELECT(  id, handler) EVT_COMMAND(id, EVENT_GALLERY_SELECT,   handler)
/// Handle EVENT_GALLERY_ACTIVATE events
#define EVT_GALLERY_ACTIVATE(id, handler) EVT_COMMAND(id, EVENT_GALLERY_ACTIVATE, handler)

// ----------------------------------------------------------------------------- : GalleryList

/// A list of items with custom drawing
/** A derived class should implement the abstract members to determine how the items look.
 */
class GalleryList : public wxPanel {
  public:
	GalleryList(Window* parent, int id, int direction = wxHORIZONTAL, bool always_focused = true);
	
	/// Select the given column
	void selectSubColumn(size_t subcol);
	/// Select the given item in the given column (or in the active column)
	void select(size_t item, size_t subcol = NO_SELECTION, bool event = true);
	/// Is there an item selected?
	inline bool hasSelection(size_t subcol = 0) const { return subcolumns[subcol].selection < itemCount(); }
	/// Is the given item selected?
	inline bool isSelected(size_t item, size_t subcol = 0) const {
		return subcol < subcolumns.size() && subcolumns[subcol].selection == item;
	}
	
	/// Redraw only the selected items
	void RefreshSelection();
	
  protected:
	static const size_t NO_SELECTION = (size_t)-1;
	size_t active_subcolumn;  ///< The active subcolumn
	wxSize item_size;         ///< The total size of a single item (over all columns)
	int direction;	          ///< Direction of the list, can be wxHORIZONTAL or wxVERTICAL
	size_t column_count;      ///< Number of major level columns (if vertical) or rows (if horizontal)
	bool always_focused;      ///< Always draw as if focused
	
	/// Redraw the list after changing the selection or the number of items
	void update();
	
	/// Return how many items there are in the list
	virtual size_t itemCount() const = 0;
	/// Draw an item
	virtual void drawItem(DC& dc, int x, int y, size_t item) = 0;
	/// How 'salient' should the selection in the given subcolumn be?
	virtual double subcolumnActivity(size_t col) const { return 0.7; }
	
	/// Filter calls to select, or apply some extra operaions
	virtual void onSelect(size_t item, size_t col, bool& changes) {}
	
	/// Return the desired size of control
	virtual wxSize DoGetBestSize() const;
	
	/// Information on the subcolumns. These are columns inside items
	struct SubColumn {
		wxPoint offset;
		wxSize  size;
		bool    can_select;
		size_t  selection;
	};
	vector<SubColumn> subcolumns;
	
  private:
	DECLARE_EVENT_TABLE();
	
	void onLeftDown  (wxMouseEvent& ev);
	void onLeftDClick(wxMouseEvent& ev);
	void onMouseWheel(wxMouseEvent& ev);
	void onChar(wxKeyEvent& ev);
	void onFocus(wxFocusEvent&);
	void onPaint(wxPaintEvent&);
	void onSize(wxSizeEvent&);
	void onScroll(wxScrollWinEvent&);
	void OnDraw(DC& dc);
	
	/// Find the item corresponding to the given location
	size_t findItem(const wxMouseEvent&) const;
	/// Find the coordinates of an item
	wxPoint itemPos(size_t item) const;
	
	/// Scroll to the given position (note: 'top' can also mean 'left')
	void scrollTo(int top, bool update_scrollbar = true);
	/// Update the scrollbar(s)
	void updateScrollbar();
	/// Redraw just a single item
	void RefreshItem(size_t item);
	
	/// First visible pixel position
	int visible_start;
	/// First no-longer-visible pixel position
	inline int visibleEnd() const {
		return visible_start + mainSize(GetClientSize());
	}
	/// Pixel position of an item
	inline int itemStart(size_t item) const {
		return (int)(item / column_count) * (mainSize(item_size) + SPACING);
	}
	inline int itemEnd(size_t item) const {
		return (int)(item / column_count + 1) * (mainSize(item_size) + SPACING) + MARGIN;
	}
	/// Main component of a size (i.e. in the direction of this list)
	inline int mainSize(wxSize s) const {
		return direction == wxHORIZONTAL ? s.x : s.y;
	}

  public:
	typedef SubColumn SubColumn_for_typeof;
  protected:
	/// Send an event
	void sendEvent(WXTYPE type);

	static const int MARGIN = 1; // margin between items (excluding border)
	static const int BORDER = 1; // border aroung items
	static const int SPACING = MARGIN + 2*BORDER; // distance between items
};

// ----------------------------------------------------------------------------- : EOF
#endif
