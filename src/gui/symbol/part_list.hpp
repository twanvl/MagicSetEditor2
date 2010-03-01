//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_PART_LIST
#define HEADER_GUI_SYMBOL_PART_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol.hpp>
#include <wx/imaglist.h>

class SymbolPartsSelection;

// ----------------------------------------------------------------------------- : Events

DECLARE_EVENT_TYPE(EVENT_PART_SELECT,   <not used>)
DECLARE_EVENT_TYPE(EVENT_PART_ACTIVATE, <not used>)

/// Handle EVENT_PART_SELECT events
#define EVT_PART_SELECT(  id, handler) EVT_COMMAND(id, EVENT_PART_SELECT,   handler)
/// Handle EVENT_PART_ACTIVATE events
#define EVT_PART_ACTIVATE(id, handler) EVT_COMMAND(id, EVENT_PART_ACTIVATE, handler)

// ----------------------------------------------------------------------------- : SymbolPartList

class SymbolPartList : public wxScrolledWindow, public SymbolView {
  public:
	SymbolPartList(Window* parent, int id, SymbolPartsSelection& selection, SymbolP symbol = SymbolP());
	
	/// Another symbol is being viewed
	virtual void onChangeSymbol();
	/// Event handler for changes to the symbol
	virtual void onAction(const Action&, bool);
	
	/// Update the control
	void update();
	/// Update only a subset of the parts
	void updateParts(const set<SymbolPartP>& parts);
	
  protected:
	virtual wxSize DoGetBestSize() const;
  private:
	SymbolPartsSelection& selection; ///< Store selection here
	int number_of_items;
	
	SymbolPartP drag;
	SymbolGroupP drag_parent, drop_parent;
	size_t drag_position, drop_position;
	bool drop_inside; // drop inside the drop parent, not at a specific position
	
	SymbolPartP typing_in;
	size_t cursor;
	
	wxImageList state_icons;
	struct Preview {
		Preview() : up_to_date(false), image(25,25) {}
		bool  up_to_date;
		Image image;
	};
	Preview symbol_preview; ///< Preview of the whole symbol
	vector<Preview> part_previews;
	
	static const int ITEM_HEIGHT = 25;
	// --------------------------------------------------- : Event handling
	DECLARE_EVENT_TABLE();
	
	void onLeftDown  (wxMouseEvent& ev);
	void onLeftDClick(wxMouseEvent& ev);
	void onLeftUp    (wxMouseEvent& ev);
	void onMotion    (wxMouseEvent& ev);
	void onChar(wxKeyEvent& ev);
	void onPaint(wxPaintEvent&);
	void onSize(wxSizeEvent&);
	void OnDraw(DC& dc);
	
	void sendEvent(int type);
	
	void drawItem(DC& dc, int x, int& i, bool parent_active, const SymbolPartP& part);
	const Image& itemPreview(int i, const SymbolPartP& part);
	const Image& symbolPreview();
	void updatePart(const set<SymbolPartP>& parts, int& i, bool parent_updated, const SymbolPartP& part);
	
	/// find item by position
	SymbolPartP findItem(int i, int x) const;
	static SymbolPartP findItem(int& i, int x, const SymbolPartP& part);
	
	/// parent of 'of' and the position of 'of' in that parent
	void findParent(const SymbolPart& of, SymbolGroupP& parent_out, size_t& pos_out);
	static bool findParent(const SymbolPart& of, const SymbolGroupP& in, SymbolGroupP& parent_out, size_t& pos_out);
	
	/// Where is the drop position?
	/**   i      = index before which the cursor is
	 *    before = is the cursor before or after the separator line?
	 *  Returns whether a drop position was found, sets drop_...
	 */
	bool findDropTarget(const SymbolPartP& parent, int& i, bool before);
	
	static int childCount(const SymbolPartP& part);
	
	void updateCaret(DC& dc, int x, int y, int h, const SymbolPartP& part);
};

// ----------------------------------------------------------------------------- : EOF
#endif
