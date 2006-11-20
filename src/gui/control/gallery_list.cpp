//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/gallery_list.hpp>
#include <gfx/gfx.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_GALLERY_SELECT);
DEFINE_EVENT_TYPE(EVENT_GALLERY_ACTIVATE);

// ----------------------------------------------------------------------------- : GalleryList

const int MARGIN = 1; // margin between items
const int BORDER = 1; // margin between items

GalleryList::GalleryList(Window* parent, int id, int direction)
	: wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | (direction == wxHORIZONTAL ? wxHSCROLL : wxVSCROLL) )
	, direction(direction)
{}

void GalleryList::update() {
	const int w = item_size.width  + MARGIN + 2*BORDER;
	const int h = item_size.height + MARGIN + 2*BORDER;
	// resize and scroll
	if (direction == wxHORIZONTAL) {
		SetVirtualSize(w * (int)itemCount() + MARGIN, h + MARGIN);
		SetScrollRate(w, 0);
	} else { // wxVERTICAL
		SetVirtualSize(w, h * (int)itemCount() + MARGIN + MARGIN);
		SetScrollRate(0, h);
	}
	// ensure selected item + its margin is visible
	if (selection < itemCount()) {
		int x, y, cw, ch;
		GetViewStart (&x,  &y);
		GetClientSize(&cw, &ch);
		cw = (cw - w + 1) / w; ch = (ch - h + 1) / h;
		RealPoint pos = itemPos(selection);
		x = min(x,      (int)selection);
		y = min(y,      (int)selection);
		x = max(x + cw, (int)selection) - cw;
		y = max(y + ch, (int)selection) - ch;
		Scroll(x,y);
	}
	// redraw
	Refresh(false);
}

size_t GalleryList::findItem(const wxMouseEvent& ev) const {
	if (direction == wxHORIZONTAL) {
		int x, w = item_size.width  + MARGIN + 2*BORDER;
		GetViewStart (&x, 0);
		return static_cast<size_t>( x + ev.GetX() / w );
	} else { // wxVERTICAL
		int y, h = item_size.height + MARGIN + 2*BORDER;
		GetViewStart (0, &y);
		return static_cast<size_t>( y + ev.GetY() / h );
	}
}

RealPoint GalleryList::itemPos(size_t item) const {
	if (direction == wxHORIZONTAL) {
		return RealPoint(item * (item_size.width + MARGIN + 2*BORDER) + MARGIN + BORDER, MARGIN + BORDER);
	} else {
		return RealPoint(MARGIN + BORDER, item * (item_size.height + MARGIN + 2*BORDER) + MARGIN + BORDER);
	}
}

// ----------------------------------------------------------------------------- : Events

void GalleryList::onLeftDown(wxMouseEvent& ev) {
	size_t item = findItem(ev);
	if (item != selection && item < itemCount()) {
		selection = item;
		update();
		sendEvent(EVENT_GALLERY_SELECT);
	}
	ev.Skip(); // focus
}

void GalleryList::onLeftDClick(wxMouseEvent& ev) {
	sendEvent(EVENT_GALLERY_ACTIVATE);
}

void GalleryList::onChar(wxKeyEvent& ev) {
	switch (ev.GetKeyCode()) {
		case WXK_LEFT:	if (direction == wxHORIZONTAL && selection > 0) {
							selection -= 1;
							update();
							sendEvent(EVENT_GALLERY_SELECT);
						} break;
		case WXK_RIGHT:	if (direction == wxHORIZONTAL && selection + 1 < itemCount()) {
							selection += 1;
							update();
							sendEvent(EVENT_GALLERY_SELECT);
						} break;
		case WXK_UP:	if (direction == wxVERTICAL   && selection > 0) {
							selection -= 1;
							update();
							sendEvent(EVENT_GALLERY_SELECT);
						} break;
		case WXK_DOWN:	if (direction == wxVERTICAL   && selection + 1 < itemCount()) {
							selection += 1;
							update();
							sendEvent(EVENT_GALLERY_SELECT);
						} break;
	}
}

wxSize GalleryList::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	const int w = item_size.width  + 2*MARGIN + 2*BORDER;
	const int h = item_size.height + 2*MARGIN + 2*BORDER;
	return wxSize(w, h) + ws - cs;
}

void GalleryList::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	DoPrepareDC(dc);
	OnDraw(dc);
}
void GalleryList::OnDraw(DC& dc) {
	int x,  y;
	int cw, ch;
	int dx, dy;
	size_t start, end; // items to draw
	// number of visble items
	GetViewStart(&x, &y);
	GetClientSize(&cw, &ch);
	if (direction == wxHORIZONTAL) {
		dx = item_size.width + MARGIN + 2*BORDER;
		dy = 0;
		start = (size_t) x;
		end   = (size_t) (start + cw / dx + 1);
	} else {
		dx = 0;
		dy = item_size.height + MARGIN + 2*BORDER;
		start = (size_t) y;
		end   = (size_t) (start + ch / dy + 1);
	}
	end = min(end, itemCount());
	// clear background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0, 0, dx * x + cw, dy * y + ch);
	// draw all visible items
	Color unselected = lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
		                    wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), 0.1);
	Color background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	for (size_t i = start ; i < end ; ++i) {
		// draw selection rectangle
		bool selected = i == selection;
		Color c = selected ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) : unselected;
		dc.SetPen(c);
		dc.SetBrush(lerp(background, c, 0.3));
		RealPoint pos = itemPos(i);
		dc.DrawRectangle(pos.x - BORDER, pos.y - BORDER, item_size.width + 2*BORDER, item_size.height + 2*BORDER);
		// draw item
		drawItem(dc, pos.x, pos.y, i, selected);
	}
}

void GalleryList::sendEvent(WXTYPE type) {
	wxCommandEvent ev(type, GetId());
	ProcessEvent(ev);
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(GalleryList, wxScrolledWindow)
	EVT_LEFT_DOWN      (GalleryList::onLeftDown)
	EVT_LEFT_DCLICK    (GalleryList::onLeftDClick)
	EVT_CHAR           (GalleryList::onChar)
	EVT_PAINT          (GalleryList::onPaint)
END_EVENT_TABLE  ()
