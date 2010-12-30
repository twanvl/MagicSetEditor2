//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_DROP_DOWN_LIST
#define HEADER_GUI_DROP_DOWN_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <wx/popupwin.h> // undocumented: wxPopupWindow

class ValueViewer;

// ----------------------------------------------------------------------------- : DropDownList

/// A popup/drop down window displaying a list of items
/** This class is an abstract base for various drop down lists */
class DropDownList : public wxPopupTransientWindow {
  public:
	~DropDownList();
	/// Create a drop down list, possibly a sub menu
	/** the viewer will be notified to redraw its drop down icon */
	DropDownList(Window* parent, bool is_submenu = false, ValueViewer* viewer = nullptr);
	
	/// Show the editor
	/** if in_place, then shows the list at the position pos.
	 *  Otherwise around the rect or the viewer rectangle
	 */
	void show(bool in_place, wxPoint pos, RealRect* rect = nullptr);
	/// Close the list, optionally send an onSelect event
	void hide(bool event, bool allow_veto = true);
	
	// --------------------------------------------------- : Parent control
	/// Takes all keyboard events from a FieldEditor
	bool onCharInParent(wxKeyEvent&);
	/// Takes a mouse event from the parent, show/hide as appropriate
	bool onMouseInParent(wxMouseEvent&, bool open_in_place);
  
  protected:
	
	/// Prepare for showing the list
	virtual void onShow() {}
	/// Do something after hiding the list
	virtual void OnDismiss();
	
	inline bool isRoot() { return parent_menu == nullptr; }
	
	// --------------------------------------------------- : Selection
	static const size_t NO_SELECTION = (size_t)-1;
	
	/// Signal that the list is closed and something is selected
	virtual void select(size_t selection) = 0;
	/// When the list is being opened, what should be selected?
	virtual size_t selection() const = 0;
	/** Should the list stay open after selecting something? */
	virtual bool stayOpen(size_t selection) const { return false; }
	
	// --------------------------------------------------- : Item information
	/// Number of items
	virtual size_t itemCount() const = 0;
	/// Text of an item
	virtual String itemText(size_t item) const = 0;
	/// Draw an icon at the specified location
	virtual void drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {}
	/// Is there a line below an item?
	virtual bool lineBelow(size_t item)        const { return false; }
	/// Should the item be highlighted?
	virtual bool highlightItem(size_t item)    const { return false; }
	/// Is the item enabled?
	virtual bool itemEnabled(size_t item)      const { return true; }
	// An extra submenu that pops up from an item, or null if there is no popup menu
	virtual DropDownList* submenu(size_t item) const { return nullptr; }
	
	// --------------------------------------------------- : Layout
	
	static const int marginW = 0;
	static const int marginH = 0;
	
	// may be changed by derived class
	int text_offset;	///< Vertical distance between top of item and text
	RealSize item_size;	///< Size of an item;
	RealSize icon_size;	///< Size of icons;
	
  private:
	// --------------------------------------------------- : Data
	
	size_t         selected_item;      ///< The item that is selected, or NO_SELECTION
	bool           mouse_down;         ///< Is the mouse pressed?
	DropDownList*  open_sub_menu;      ///< The sub menu that is currently shown, if any
	DropDownList*  parent_menu;        ///< The parent menu, only applies to sub menus
	ValueViewer*   viewer;             ///< The parent viewer object (optional)
	bool           close_on_mouse_out; ///< Was the list kept open after selecting a choice, if so, be eager to close it
	int            visible_start;      ///< First visible pixel
	
	// --------------------------------------------------- : Events
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
	void onLeftDown(wxMouseEvent&);
	void onLeftUp  (wxMouseEvent&);
	void onMotion(wxMouseEvent&);
	void onMouseLeave(wxMouseEvent&);
	void onMouseWheel(wxMouseEvent& ev);
	void onScroll(wxScrollWinEvent&);
	
	// --------------------------------------------------- : Privates
	
	/// Return the y coordinate of an item (in scrolled coordinates)
	int itemPosition(size_t item) const;
	
	void realHide();
	void hideSubMenu();
	bool showSubMenu();
	bool showSubMenu(size_t item, int y);
	bool selectItem(size_t item);
	void ensureSelectedItemVisible();
	void scrollTo(int pos);
	
	void draw(DC& dc);
	void drawItem(DC& dc, int y, size_t item);
	
  protected:
	virtual void redrawArrowOnParent(); // allow override
};

// ----------------------------------------------------------------------------- : EOF
#endif
