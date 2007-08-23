//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_viewer.hpp>
#include <data/stylesheet.hpp>
#include <data/settings.hpp>
#include <render/value/viewer.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_SIZE_CHANGE);

// ----------------------------------------------------------------------------- : CardViewer

CardViewer::CardViewer(Window* parent, int id, long style)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, style | wxNO_FULL_REPAINT_ON_RESIZE)
	, up_to_date(false)
{}

wxSize CardViewer::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	if (set) {
		return (wxSize)getRotation().getExternalSize() + ws - cs;
	}
	return cs;
}

void CardViewer::redraw(const ValueViewer& v) {
	if (drawing) return;
	up_to_date = false;
	RefreshRect(getRotation().tr(v.boundingBox()), false);
}

void CardViewer::onChange() {
	redraw();
}

void CardViewer::redraw() {
	if (drawing) return;
	up_to_date = false;
	Refresh(false);
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
	// scrolling
//	int dx = GetScrollPos(wxHORIZONTAL), dy = GetScrollPos(wxVERTICAL);
//	dc.SetDeviceOrigin(-dx, -dy);
	wxRegion clip = GetUpdateRegion();
//	clip.Offset(dx, dy);
	dc.SetClippingRegion(clip);
	// draw
	if (!up_to_date) {
		up_to_date = true;
		draw(dc);
	}
}

void CardViewer::drawViewer(RotatedDC& dc, ValueViewer& v) {
	if (shouldDraw(v)) v.draw(dc);
}

bool CardViewer::shouldDraw(const ValueViewer& v) const {
//	int dx = GetScrollPos(wxHORIZONTAL), dy = GetScrollPos(wxVERTICAL);
//	wxRegion clip = GetUpdateRegion();
//	clip.Offset(dx, dy);
	return GetUpdateRegion().Contains(getRotation().tr(v.boundingBox().toRect()).toRect()) != wxOutRegion;
}

// helper class for overdrawDC()
class CardViewer::OverdrawDC_aux : private wxClientDC {
  protected:
	wxBufferedDC bufferedDC;
	
	OverdrawDC_aux(CardViewer* window)
		: wxClientDC(window)
	{
		bufferedDC.Init((wxClientDC*)this, window->buffer);
	}
};
class CardViewer::OverdrawDC : private OverdrawDC_aux, public RotatedDC {
  public:
	OverdrawDC(CardViewer* window)
		: OverdrawDC_aux(window)
		, RotatedDC(bufferedDC, window->getRotation(), QUALITY_LOW)
	{}
};

shared_ptr<RotatedDC> CardViewer::overdrawDC() {
	#ifdef _DEBUG
		// don't call from onPaint
		assert(!inOnPaint());
	#endif
	return shared_ptr<RotatedDC>(new OverdrawDC(this));
}

Rotation CardViewer::getRotation() const {
	// Same as DataViewer::getRotation, only taking into account scrolling
	if (!stylesheet) stylesheet = set->stylesheet;
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	int dx = GetScrollPos(wxHORIZONTAL), dy = GetScrollPos(wxVERTICAL);
	return Rotation(ss.card_angle(), stylesheet->getCardRect().move(-dx,-dy,0,0), ss.card_zoom(), true);
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(CardViewer, wxControl)
	EVT_PAINT      (CardViewer::onPaint)
END_EVENT_TABLE  ()
