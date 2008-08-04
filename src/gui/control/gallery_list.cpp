//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/gallery_list.hpp>
#include <gfx/gfx.hpp>
#include <wx/dcbuffer.h>

DECLARE_TYPEOF_COLLECTION(GalleryList::Column_for_typeof);

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_GALLERY_SELECT);
DEFINE_EVENT_TYPE(EVENT_GALLERY_ACTIVATE);

// ----------------------------------------------------------------------------- : GalleryList

GalleryList::GalleryList(Window* parent, int id, int direction, bool always_focused)
	: wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxWANTS_CHARS | (direction == wxHORIZONTAL ? wxHSCROLL : wxVSCROLL) )
	, active_column(0)
	, direction(direction)
	, always_focused(always_focused)
	, visible_start(0)
{
	Column col;
	col.can_select = true;
	col.selection  = NO_SELECTION;
	columns.push_back(col);
}

void GalleryList::selectColumn(size_t column) {
	if (column >= columns.size()) return;
	if (!columns[column].can_select) return;
	if (active_column == column) return;
	RefreshItem(columns[active_column].selection);
	RefreshItem(columns[column       ].selection);
	active_column = column;
}

void GalleryList::select(size_t item, size_t column, bool event) {
	if (item >= itemCount()) return;
	// select column
	size_t old_active_column = active_column;
	selectColumn(column);
	Column& col = columns[active_column];
	// filter?
	bool changes = col.selection != item;
	onSelect(item, old_active_column, changes);
	// select
	size_t old_sel = col.selection;
	col.selection = item;
	changes |= col.selection != old_sel;
	// ensure visible
	if (itemStart(col.selection) < visible_start) {
		scrollTo(itemStart(col.selection));
	} else if (itemEnd(col.selection) > visibleEnd()) {
		scrollTo(itemEnd(col.selection) + visible_start - visibleEnd());
	} else if (col.selection != old_sel) {
		RefreshItem(old_sel);
		RefreshItem(col.selection);
	}
	// send event
	if (event && changes) {
		sendEvent(EVENT_GALLERY_SELECT);
	}
}

void GalleryList::update() {
	select(columns[active_column].selection);
	updateScrollbar();
	Refresh(false);
}

size_t GalleryList::findItem(const wxMouseEvent& ev) const {
	int x = visible_start + (direction == wxHORIZONTAL ? ev.GetX() : ev.GetY());
	int w = mainSize(item_size) + SPACING;
	return static_cast<size_t>( max(0, x - MARGIN) / w ); 
}

wxPoint GalleryList::itemPos(size_t item) const {
	if (direction == wxHORIZONTAL) {
		return wxPoint((int)item * (item_size.x + SPACING) + MARGIN + BORDER - visible_start, MARGIN + BORDER);
	} else {
		return wxPoint(MARGIN + BORDER, (int)item * (item_size.y + SPACING) + MARGIN + BORDER - visible_start);
	}
}

// ----------------------------------------------------------------------------- : Scrolling & sizing

void GalleryList::scrollTo(int top, bool update_scrollbar) {
	wxSize cs = GetClientSize();
	int total_height = itemEnd(itemCount() - 1);
	top = min(total_height - mainSize(cs), top);
	top = max(0, top);
	// scroll
	if (top == visible_start) return;
	visible_start = top;
	if (update_scrollbar) {
		// scroll bar
		updateScrollbar();
		// scroll actual window content
		Refresh(false);
	}
}

void GalleryList::updateScrollbar() {
	scrollTo(visible_start, false);
    // how many lines fit on the screen?
    int screen_height = mainSize(GetClientSize());
	int total_height  = itemEnd(itemCount() - 1);
	// set the scrollbar parameters to reflect this
    SetScrollbar(direction, visible_start, screen_height, total_height);
}

void GalleryList::RefreshItem(size_t item) {
	if (item >= itemCount()) return;
	RefreshRect(wxRect(itemPos(item),item_size).Inflate(BORDER,BORDER), false);
}
void GalleryList::RefreshSelection() {
	FOR_EACH(col,columns) RefreshItem(col.selection);
}

void GalleryList::onScroll(wxScrollWinEvent& ev) {
    wxEventType type = ev.GetEventType();
    if (type == wxEVT_SCROLLWIN_TOP) {
		scrollTo(0);
	} else if (type == wxEVT_SCROLLWIN_BOTTOM) {
		scrollTo(INT_MAX);
	} else if (type == wxEVT_SCROLLWIN_LINEUP) {
		scrollTo(visible_start - (mainSize(item_size) + SPACING));
	} else if (type == wxEVT_SCROLLWIN_LINEDOWN) {
		scrollTo(visible_start + (mainSize(item_size) + SPACING));
	} else if (type == wxEVT_SCROLLWIN_PAGEUP) {
		scrollTo(visible_start - visibleEnd() + mainSize(item_size));
	} else if (type == wxEVT_SCROLLWIN_PAGEDOWN) {
		scrollTo(visibleEnd() - mainSize(item_size));
	} else {
		scrollTo(ev.GetPosition());
    }
}

void GalleryList::onSize(wxSizeEvent& ev) {
	update();
	ev.Skip();
}

void GalleryList::onMouseWheel(wxMouseEvent& ev) {
	scrollTo(visible_start - (mainSize(item_size) + SPACING) * ev.GetWheelRotation() / ev.GetWheelDelta());
}

// ----------------------------------------------------------------------------- : Events

void GalleryList::onLeftDown(wxMouseEvent& ev) {
	size_t item = findItem(ev);
	if (item < itemCount()) {
		// find column
		wxPoint pos = itemPos(item);
		int x = ev.GetX() - pos.x;
		int y = ev.GetY() - pos.y;
		size_t column = active_column;
		for (size_t j = 0 ; columns.size() ; ++j) {
			Column& col = columns[j];
			if (x >= col.offset.x && y >= col.offset.y && x < col.size.x + col.offset.x && y < col.size.y + col.offset.y) {
				// clicked on this column
				column = j;
				break;
			}
		}
		select(item, column);
	}
	ev.Skip(); // focus
}

void GalleryList::onLeftDClick(wxMouseEvent& ev) {
	sendEvent(EVENT_GALLERY_ACTIVATE);
}

void GalleryList::onChar(wxKeyEvent& ev) {
	Column& col = columns[active_column];
	switch (ev.GetKeyCode()) {
		case WXK_LEFT:
			if (direction == wxHORIZONTAL) {
				select(col.selection - 1);
			} else {
				selectColumn(active_column - 1);
			}
			break;
		case WXK_RIGHT:
			if (direction == wxHORIZONTAL) {
				select(col.selection + 1);
			} else {
				selectColumn(active_column + 1);
			}
			break;
		case WXK_UP:
			if (direction == wxVERTICAL) {
				select(col.selection - 1);
			} else {
				selectColumn(active_column - 1);
			}
			break;
		case WXK_DOWN:
			if (direction == wxVERTICAL) {
				select(col.selection + 1);
			} else {
				selectColumn(active_column + 1);
			}
			break;
		case WXK_TAB: {
			// send a navigation event to our parent, to select another control
			// we need this because tabs of wxWANTS_CHARS
			wxNavigationKeyEvent nev;
			nev.SetDirection(!ev.ShiftDown());
			GetParent()->ProcessEvent(nev);
			} break;
		case WXK_RETURN: {
			// same thing: press dialog box default button
			wxButton* btn = wxDynamicCast(wxDynamicCast(GetParent(), wxTopLevelWindow)->GetDefaultItem(), wxButton);
			if ( btn && btn->IsEnabled() ) {
				// if we do have a default button, do press it
				wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, btn->GetId());
				btn->ProcessEvent(evt);
			}
			}break;
			
	}
}

wxSize GalleryList::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	const int w = item_size.x + 2*MARGIN + 2*BORDER;
	const int h = item_size.y + 2*MARGIN + 2*BORDER;
	return wxSize(w, h) + ws - cs;
}

void GalleryList::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	try {
		OnDraw(dc);
	} CATCH_ALL_ERRORS(false); // don't show message boxes in onPaint!
}
void GalleryList::OnDraw(DC& dc) {
	wxSize cs = GetClientSize();
	size_t start, end; // items to draw
	// number of visble items
	start = (size_t) max(0, visible_start / (mainSize(item_size) + SPACING));
	end   = (size_t) max(0, visibleEnd()  / (mainSize(item_size) + SPACING) + 1);
	end = min(end, itemCount());
	// clear background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0, 0, cs.x, cs.y);
	// draw all visible items
	Color unselected = lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
		                    wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), 0.1);
	Color background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	bool has_focus = always_focused || FindFocus() == this;
	for (size_t i = start ; i < end ; ++i) {
		wxPoint pos = itemPos(i);
		// draw selection rectangle
		for (size_t j = 0 ; j < columns.size() ; ++j) {
			const Column& col = columns[j];
			bool selected = i == col.selection;
			Color c = selected ? ( has_focus && j == active_column
			                            ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)
			                            : lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
			                                   wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), columnActivity(j))
			                     )
			                   : unselected;
			dc.SetPen(c);
			dc.SetBrush(saturate(lerp(background, c, 0.3), selected ? 0.5 : 0));
			dc.DrawRectangle(pos.x + col.offset.x - BORDER, pos.y + col.offset.y - BORDER,
			                 col.size.x + 2*BORDER, col.size.y + 2*BORDER);
		}
		// draw item
		drawItem(dc, pos.x, pos.y, i);
	}
}

void GalleryList::onFocus(wxFocusEvent&) {
	if (!always_focused) Refresh(false);
}

void GalleryList::sendEvent(WXTYPE type) {
	wxCommandEvent ev(type, GetId());
	ProcessEvent(ev);
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(GalleryList, wxPanel)
	EVT_LEFT_DOWN      (GalleryList::onLeftDown)
	EVT_LEFT_DCLICK    (GalleryList::onLeftDClick)
	EVT_MOUSEWHEEL     (GalleryList::onMouseWheel)
	EVT_CHAR           (GalleryList::onChar)
	EVT_SET_FOCUS      (GalleryList::onFocus)
	EVT_KILL_FOCUS     (GalleryList::onFocus)
	EVT_PAINT          (GalleryList::onPaint)
	EVT_SIZE           (GalleryList::onSize)
	EVT_SCROLLWIN      (GalleryList::onScroll)
END_EVENT_TABLE  ()
