//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_viewer.hpp>
#include <data/stylesheet.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_SIZE_CHANGE);

// ----------------------------------------------------------------------------- : CardViewer

CardViewer::CardViewer(Window* parent, int id, long style)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, style)
	, up_to_date(false)
{}

wxSize CardViewer::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	if (set) {
		return (wxSize)getRotation().getExternalSize() + ws - cs;
	}
	return cs;
}

void CardViewer::onChange() {
	Refresh(false);
	up_to_date = false;
}

void CardViewer::onChangeSize() {
	wxSize ws = GetSize(), cs = GetClientSize();
	wxSize desired_cs = (wxSize)getRotation().getExternalSize() + ws - cs;
	if (desired_cs != cs) {
		wxCommandEvent ev(EVENT_SIZE_CHANGE, GetId());
		ProcessEvent(ev);
	}
}


#ifdef _DEBUG
	DECLARE_DYNAMIC_ARG(bool, inOnPaint);
	IMPLEMENT_DYNAMIC_ARG(bool, inOnPaint, false);
#endif

void CardViewer::onPaint(wxPaintEvent&) {
	#ifdef _DEBUG
		// we don't want recursion
		assert(!inOnPaint());
		WITH_DYNAMIC_ARG(inOnPaint, true);
	#endif
	wxSize cs = GetClientSize();
	if (!buffer.Ok() || buffer.GetWidth() != cs.GetWidth() || buffer.GetHeight() != cs.GetHeight()) {
		buffer = Bitmap(cs.GetWidth(), cs.GetHeight());
		up_to_date = false;
	}
	wxBufferedPaintDC dc(this, buffer);
	if (!up_to_date) {
		up_to_date = true;
		dc.BeginDrawing();
		draw(dc);
		dc.EndDrawing();
	}
}

// helper class for overdrawDC()
class CardViewer::OverdrawDC : private wxClientDC, public wxBufferedDC {
  public:
	OverdrawDC(CardViewer* window)
		: wxClientDC(window)
	{
		wxBufferedDC::Init((wxClientDC*)this, window->buffer);
		wxBufferedDC::BeginDrawing();
	}
	~OverdrawDC() {
		wxBufferedDC::EndDrawing();
	}
};

shared_ptr<DC> CardViewer::overdrawDC() {
	#ifdef _DEBUG
		// don't call from onPaint
		assert(!inOnPaint());
	#endif
	return shared_ptr<DC>((wxBufferedDC*)(new OverdrawDC(this)));
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(CardViewer, wxControl)
	EVT_PAINT      (CardViewer::onPaint)
END_EVENT_TABLE  ()
