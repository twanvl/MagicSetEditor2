//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/gallery_list.hpp>

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_GALLERY_SELECT);
DEFINE_EVENT_TYPE(EVENT_GALLERY_ACTIVATE);

// ----------------------------------------------------------------------------- : GalleryList

const int MARGIN = 2; // margin around items

GalleryList::GalleryList(Window* parent, int id, int direction)
	: wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
	, direction(direction)
{}

void GalleryList::update() {
	const int w = item_size.GetWidth()  + 2 * MARGIN;
	const int h = item_size.GetHeight() + 2 * MARGIN;
	// resize and scroll
	if (direction == wxHORIZONTAL) {
		// resize window
		SetVirtualSize(w, h * (int)itemCount());
		SetScrollRate(w, 0);
		// ensure selected item is visible
		if (selection < itemCount()) {
			int x, cw;
			GetViewStart (&x,  0);
			GetClientSize(&cw, 0);
			cw /= w;
			if ((int)selection < x) {
				Scroll((int)selection, -1); // scroll up
			} else if ((int)selection >= x + cw) {
				Scroll((int)selection - cw - 1, -1); // scroll up
			}
		}
	} else { // wxVERTICAL
		// resize window
		SetVirtualSize(w * (int)itemCount(), h);
		SetScrollRate(0, h);
		// ensure selected item is visible
		if (selection < itemCount()) {
			int y, ch;
			GetViewStart (0, &y);
			GetClientSize(0, &ch);
			ch /= w;
			if ((int)selection < y) {
				Scroll((int)selection, -1); // scroll up
			} else if ((int)selection >= y + ch) {
				Scroll((int)selection - ch - 1, -1); // scroll down
			}
		}
	}
	// redraw
	Refresh(false);
}

size_t GalleryList::findItem(const wxMouseEvent& ev) {
	if (direction == wxHORIZONTAL) {
		int x, w = item_size.GetWidth()  + 2 * MARGIN;
		GetViewStart (&x, 0);
		return static_cast<size_t>( x + ev.GetX() / w );
	} else { // wxVERTICAL
		int y, h = item_size.GetHeight() + 2 * MARGIN;
		GetViewStart (0, &y);
		return static_cast<size_t>( y + ev.GetY() / h );
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
}

void GalleryList::onLeftDClick(wxMouseEvent& ev) {
	sendEvent(EVENT_GALLERY_ACTIVATE);
}

void GalleryList::onKeyDown(wxKeyEvent& ev) {
	switch (ev.GetKeyCode()) {
		case WXK_LEFT:	if (direction == wxHORIZONTAL && selection > 0) {
							selection -= 1;
							update();
						} break;
		case WXK_RIGHT:	if (direction == wxHORIZONTAL && selection + 1 < itemCount()) {
							selection += 1;
							update();
						} break;
		case WXK_UP:	if (direction == wxVERTICAL   && selection > 0) {
							selection -= 1;
							update();
						} break;
		case WXK_DOWN:	if (direction == wxVERTICAL   && selection + 1 < itemCount()) {
							selection += 1;
							update();
						} break;
	}
}

// Linear interpolation between colors
Color lerp(const Color& a, const Color& b, double t) {
	return Color(a.Red()   + (b.Red()   - a.Red()  ) * t,
	             a.Green() + (b.Green() - a.Green()) * t,
	             a.Blue()  + (b.Blue()  - a.Blue() ) * t);
}

wxSize GalleryList::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	const int w = item_size.GetWidth()  + 2 * MARGIN;
	const int h = item_size.GetHeight() + 2 * MARGIN;
	return wxSize(w, h) + ws - cs;
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
		dx = item_size.GetWidth() + 2 * MARGIN;
		dy = 0;
		start = (size_t) x;
		end   = (size_t) (start + cw / dx + 1);
	} else {
		dx = 0;
		dy = item_size.GetHeight() + 2 * MARGIN;
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
		dc.DrawRectangle((int)i * dx + 1, (int)i * dy + 1, item_size.GetWidth() + 2, item_size.GetHeight() + 2);
		// draw item
		drawItem(dc, (int)i * dx + MARGIN, (int)i * dy + MARGIN, i, selected);
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
	EVT_KEY_DOWN       (GalleryList::onKeyDown)
END_EVENT_TABLE  ()
