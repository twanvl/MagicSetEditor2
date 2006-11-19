//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/graph.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : Graph


// ----------------------------------------------------------------------------- : GraphControl

GraphControl::GraphControl(Window* parent, int id)
	: wxControl(parent, id)
{}

void GraphControl::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
}

void GraphControl::onSize(wxSizeEvent&) {
	Refresh(false);
}

void GraphControl::onMouseDown(wxMouseEvent& ev) {
	if (!graph) return;
	wxSize cs = GetClientSize();
	graph->findItem(RealPoint((double)ev.GetX()/cs.GetWidth(), (double)ev.GetY()/cs.GetHeight()), current_item);
}

BEGIN_EVENT_TABLE(GraphControl, wxControl)
	EVT_PAINT		(GraphControl::onPaint)
	EVT_SIZE		(GraphControl::onSize)
	EVT_LEFT_DOWN	(GraphControl::onMouseDown)
END_EVENT_TABLE  ()
