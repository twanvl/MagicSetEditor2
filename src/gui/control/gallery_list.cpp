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

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_GALLERY_SELECT);
DEFINE_EVENT_TYPE(EVENT_GALLERY_ACTIVATE);

// ----------------------------------------------------------------------------- : GalleryList

GalleryList::GalleryList(Window* parent, int id, int direction, bool always_focused)
	: wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxWANTS_CHARS | (direction == wxHORIZONTAL ? wxHSCROLL : wxVSCROLL) )
	, selection(NO_SELECTION)
	, direction(direction)
	, always_focused(always_focused)
	, visible_start(0)
{}

void GalleryList::select(size_t item, bool event) {
	if (item >= itemCount()) return;
	// select
	size_t old_sel = selection;
	selection = item;
	// ensure visible
	if (itemStart(selection) < visible_start) {
		scrollTo(itemStart(selection));
	} else if (itemEnd(selection) > visibleEnd()) {
		scrollTo(itemEnd(selection) + visible_start - visibleEnd());
	} else {
		RefreshItem(old_sel);
		RefreshItem(selection);
	}
	// send event
	if (event && selection != old_sel) {
		sendEvent(EVENT_GALLERY_SELECT);
	}
}

void GalleryList::update() {
	select(selection);
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
	if (item != selection && item < itemCount()) {
		select(item);
	}
	ev.Skip(); // focus
}

void GalleryList::onLeftDClick(wxMouseEvent& ev) {
	sendEvent(EVENT_GALLERY_ACTIVATE);
}

void GalleryList::onChar(wxKeyEvent& ev) {
	switch (ev.GetKeyCode()) {
		case WXK_LEFT:	if (direction == wxHORIZONTAL) {
							select(selection - 1);
						} break;
		case WXK_RIGHT:	if (direction == wxHORIZONTAL) {
							select(selection + 1);
						} break;
		case WXK_UP:	if (direction == wxVERTICAL) {
							select(selection - 1);
						} break;
		case WXK_DOWN:	if (direction == wxVERTICAL) {
							select(selection + 1);
						} break;
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
	OnDraw(dc);
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
	for (size_t i = start ; i < end ; ++i) {
		// draw selection rectangle
		bool selected = i == selection;
		Color c = selected ? ( always_focused || FindFocus() == this
		                            ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)
		                            : lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
		                                   wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), 0.7)
		                     )
		                   : unselected;
		dc.SetPen(c);
		dc.SetBrush(saturate(lerp(background, c, 0.3), selected ? 0.5 : 0));
		wxPoint pos = itemPos(i);
		dc.DrawRectangle(pos.x - BORDER, pos.y - BORDER, item_size.x + 2*BORDER, item_size.y + 2*BORDER);
		// draw item
		drawItem(dc, pos.x, pos.y, i, selected);
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
