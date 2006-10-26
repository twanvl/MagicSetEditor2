//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_viewer.hpp>
#include <data/stylesheet.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : CardViewer

CardViewer::CardViewer(Window* parent, int id, int style)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, style)
{}

wxSize CardViewer::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	return wxSize(set->stylesheet->card_width, set->stylesheet->card_height) + ws - cs;
}

void CardViewer::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(CardViewer, wxControl)
	EVT_PAINT      (CardViewer::onPaint)
END_EVENT_TABLE  ()
