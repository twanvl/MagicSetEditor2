//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_viewer.hpp>
#include <data/stylesheet.hpp>
#include <data/settings.hpp>
#include <render/value/viewer.hpp>
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
		if (!stylesheet) stylesheet = set->stylesheet;
		StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
		wxSize size(int(stylesheet->card_width * ss.card_zoom()), int(stylesheet->card_height * ss.card_zoom()));
		if (sideways(ss.card_angle())) swap(size.x, size.y);
		return size + ws - cs;
	}
	return cs;
}

void CardViewer::redraw(const ValueViewer& v) {
	// Don't refresh if we OR ANOTHER CardViewer is drawing
	// drawing another viewer causes styles to be updated for its active card, which may be different,
	// causing the two viewers to continously refresh.
	if (drawing_card()) return;
	up_to_date = false;
	RefreshRect(getRotation().trRectToBB(v.boundingBox()), false);
}

void CardViewer::onChange() {
	redraw();
}

void CardViewer::redraw() {
	if (drawing_card()) return;
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
		if (inOnPaint()) {
			wxTrap();
		}
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
		try {
			draw(dc);
		} CATCH_ALL_ERRORS(false); // don't show message boxes in onPaint!
	}
}

void CardViewer::drawViewer(RotatedDC& dc, ValueViewer& v) {
	if (shouldDraw(v)) v.draw(dc);
}

bool CardViewer::shouldDraw(const ValueViewer& v) const {
//	int dx = GetScrollPos(wxHORIZONTAL), dy = GetScrollPos(wxVERTICAL);
//	wxRegion clip = GetUpdateRegion();
//	clip.Offset(dx, dy);
	return GetUpdateRegion().Contains(getRotation().trRectToBB(v.boundingBox().toRect()).toRect()) != wxOutRegion;
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
		if (inOnPaint()) {
			wxTrap();
		}
	#endif
	return shared_ptr<RotatedDC>(new OverdrawDC(this));
}

Rotation CardViewer::getRotation() const {
	// Same as DataViewer::getRotation, only taking into account scrolling
	if (!stylesheet) stylesheet = set->stylesheet;
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	int dx = GetScrollPos(wxHORIZONTAL), dy = GetScrollPos(wxVERTICAL);
	return Rotation(ss.card_angle(), stylesheet->getCardRect().move(-dx,-dy,0,0), ss.card_zoom(), 1.0, ROTATION_ATTACH_TOP_LEFT);
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(CardViewer, wxControl)
	EVT_PAINT      (CardViewer::onPaint)
END_EVENT_TABLE  ()
