//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
	
	/// Select the given item
	void select(size_t item, bool event = true);
	/// Is there an item selected?
	inline bool hasSelection() const { return selection < itemCount(); }
	
  protected:
	static const size_t NO_SELECTION = (size_t)-1;
	size_t selection;      ///< The selected item, or NO_SELECTION if there is no selection
	wxSize item_size;      ///< The size of a single item
	int direction;	       ///< Direction of the list, can be wxHORIZONTAL or wxVERTICAL
	bool always_focused;   ///< Always draw as if focused
	
	/// Redraw the list after changing the selection or the number of items
	void update();
	
	/// Return how many items there are in the list
	virtual size_t itemCount() const = 0;
	/// Draw an item
	virtual void drawItem(DC& dc, int x, int y, size_t item, bool selected) = 0;
	
	/// Return the desired size of control
	virtual wxSize DoGetBestSize() const;
	
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
	void GalleryList::scrollTo(int top, bool update_scrollbar = true);
	/// Update the scrollbar(s)
	void GalleryList::updateScrollbar();
	/// Redraw just a single item
	void GalleryList::RefreshItem(size_t item);
	
	/// First visible pixel position
	int visible_start;
	/// First no-longer-visible pixel position
	inline int GalleryList::visibleEnd() const;
	/// Pixel position of an item
	inline int GalleryList::itemStart(size_t item) const;
	inline int GalleryList::itemEnd(size_t item) const;
	/// Main component of a size (i.e. in the direction of this list)
	inline int GalleryList::mainSize(wxSize s) const;

  protected:
	/// Send an event
	void sendEvent(WXTYPE type);
};

// ----------------------------------------------------------------------------- : EOF
#endif
