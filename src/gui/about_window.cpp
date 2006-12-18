//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/about_window.hpp>
#include <gui/util.hpp>
#include <util/version.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : About window

AboutWindow::AboutWindow(Window* parent)
	: wxDialog(parent, wxID_ANY, _TITLE_("about"), wxDefaultPosition, wxSize(510,340), wxCLIP_CHILDREN | wxDEFAULT_DIALOG_STYLE)
	, logo (load_resource_image(_("ABOUT")))
	, logo2(load_resource_image(_("TWO")))
{
	// init controls
	wxButton* ok_button = new HoverButton(this, wxID_OK, _("BTN_OK"));
	wxSize bs = ok_button->GetSize(), ws = GetClientSize();
	ok_button->Move(ws.GetWidth() - bs.GetWidth(),  ws.GetHeight() - bs.GetHeight()); // align bottom right
}

void AboutWindow::onPaint(wxPaintEvent& ev) {
	wxBufferedPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}

void AboutWindow::draw(DC& dc) {
	wxSize ws = GetClientSize();
	// draw background
	dc.SetPen  (*wxTRANSPARENT_PEN);
	dc.SetBrush(Color(240,247,255));
	dc.DrawRectangle(0, 0, ws.GetWidth(), ws.GetHeight());
	// draw logo
	dc.DrawBitmap(logo,  (ws.GetWidth() -  logo.GetWidth()) / 2, 5);
	dc.DrawBitmap(logo2,  ws.GetWidth() - logo2.GetWidth(),      ws.GetHeight() - logo2.GetHeight());
	// draw version box
	dc.SetPen  (wxPen(Color(184,29,19), 2));
	dc.SetBrush(Color(114,197,224));
	dc.DrawRectangle(28, 104, 245, 133);
	dc.SetTextBackground(Color(114,197,224));
	dc.SetTextForeground(Color(0,0,0));
	// draw version info
	dc.SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _("Arial")));
	dc.DrawText(_("Version: ") + app_version.toString() + version_suffix, 34, 110);
	dc.DrawText(_("This is a development version!"), 34, 130);
	dc.DrawText(_("   it probably contains lots bugs,"), 34, 147);
	dc.DrawText(_("   it misses some very important features,"), 34, 164);
	dc.DrawText(_("   don't use it for anything important"), 34, 181);
	dc.DrawText(_("Copyright \xA9 2006 Twan van Laarhoven"), 34, 214);
}

BEGIN_EVENT_TABLE(AboutWindow, wxDialog)
	EVT_PAINT      (AboutWindow::onPaint)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Button with image and hover effect


HoverButton::HoverButton(Window* parent, int id, const String& name)
	: normal(load_resource_image(name + _("_NORMAL")))
	, hover (load_resource_image(name + _("_HOVER")))
{
	Create(parent, id, normal, wxDefaultPosition, wxSize(normal.GetWidth(), normal.GetHeight()), 0);
}

void HoverButton::onMouseEnter(wxMouseEvent&) {
	SetBitmapLabel(hover);
	Refresh(false);
}
void HoverButton::onMouseLeave(wxMouseEvent&) {
	SetBitmapLabel(normal);
	Refresh(false);
}
void HoverButton::onFocus(wxFocusEvent&) {}
void HoverButton::onKillFocus(wxFocusEvent&) {}

void HoverButton::onPaint(wxPaintEvent&) {
	wxPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}
void HoverButton::draw(DC& dc) {
	// clear background (for transparent button images)
	wxSize ws = GetClientSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(Color(240,247,255));
	dc.DrawRectangle(0, 0, ws.GetWidth(), ws.GetHeight());
	// draw button
	dc.DrawBitmap(GetBitmapLabel(), 0, 0);
}

BEGIN_EVENT_TABLE(HoverButton, wxBitmapButton)
	EVT_ENTER_WINDOW   (HoverButton::onMouseEnter)
	EVT_LEAVE_WINDOW   (HoverButton::onMouseLeave)
	EVT_PAINT          (HoverButton::onPaint)
	EVT_SET_FOCUS      (HoverButton::onFocus)
	EVT_KILL_FOCUS     (HoverButton::onKillFocus)
END_EVENT_TABLE  ()
