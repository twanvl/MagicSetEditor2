//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_PART_LIST
#define HEADER_GUI_SYMBOL_PART_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol.hpp>

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
	SymbolPartList(Window* parent, int id, set<SymbolPartP>& selection, SymbolP symbol = SymbolP());
	
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
	set<SymbolPartP>& selection; ///< Store selection here
	
	SymbolPartP mouse_down_on;
	int drop_position;
	int number_of_items;
	
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
	
	SymbolPartP findItem(int i) const;
	static SymbolPartP findItem(int& i, const SymbolPartP& part);
	
	static int childCount(const SymbolPartP& part);
	
	void updateCaret(DC& dc, int x, int y, int h, const SymbolPartP& part);
};

// ----------------------------------------------------------------------------- : EOF
#endif
