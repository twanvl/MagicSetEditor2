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

CardViewer::CardViewer(Window* parent, int id, long style)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, style)
	, up_to_date(false)
{}

wxSize CardViewer::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	if (set) {
		StyleSheetP stylesheet = set->stylesheetFor(card);
		if (stylesheet) {
			return wxSize(stylesheet->card_width, stylesheet->card_height) + ws - cs;
		}
	}
	return cs;
}

void CardViewer::onChange() {
	Refresh(false);
	up_to_date = false;
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
