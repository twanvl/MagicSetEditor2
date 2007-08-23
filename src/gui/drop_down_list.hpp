//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_DROP_DOWN_LIST
#define HEADER_GUI_DROP_DOWN_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <wx/popupwin.h> // undocumented: wxPopupWindow

class ValueViewer;
class DropDownHider;

// ----------------------------------------------------------------------------- : DropDownList

/// A popup/drop down window displaying a list of items
/** This class is an abstract base for various drop down lists */
class DropDownList : public wxPopupWindow {
  public:
	~DropDownList();
	/// Create a drop down list, possibly a sub menu
	/** the viewer will be notified to redraw its drop down icon */
	DropDownList(Window* parent, bool is_submenu = false, ValueViewer* viewer = nullptr);
	
	/// Show the editor
	/** if in_place, then shows the list at the position pos */
	void show(bool in_place, wxPoint pos);
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
	
	// --------------------------------------------------- : Selection
	static const size_t NO_SELECTION = (size_t)-1;
	
	/// Signal that the list is closed and something is selected
	virtual void select(size_t selection) = 0;
	/// When the list is being opened, what should be selected?
	virtual size_t selection() const = 0;
	/** Should the list stay open after selecting something? */
	virtual bool stayOpen() const { return false; }
	
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
	
	static const int marginW = 1;
	static const int marginH = 1;
	
	// may be changed by derived class
	int text_offset;	///< Vertical distance between top of item and text
	RealSize item_size;	///< Size of an item;
	RealSize icon_size;	///< Size of icons;
	
  private:
	// --------------------------------------------------- : Data
	
	size_t        selected_item;	///< The item that is selected, or NO_SELECTION
	bool          mouse_down;		///< Is the mouse pressed?
	DropDownList* open_sub_menu;	///< The sub menu that is currently shown, if any
	DropDownList* parent_menu;		///< The parent menu, only applies to sub menus
	ValueViewer*  viewer;			///< The parent viewer object (optional)
	DropDownHider* hider, *hider2;	///< Class to hide this window when we lose focus
		
	// --------------------------------------------------- : Events
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
	void onLeftDown(wxMouseEvent&);
	void onLeftUp  (wxMouseEvent&);
  protected:
	virtual void onMotion(wxMouseEvent&); // allow override
  private:
	
	// --------------------------------------------------- : Privates
	
	/// Return the y coordinate of an item
	int itemPosition(size_t item) const;
	
	void realHide();
	void hideSubMenu();
	bool showSubMenu();
	bool showSubMenu(size_t item, int y);
	
	void draw(DC& dc);
	void drawItem(DC& dc, int y, size_t item);
	
	void redrawArrowOnParent();
};

// ----------------------------------------------------------------------------- : EOF
#endif
