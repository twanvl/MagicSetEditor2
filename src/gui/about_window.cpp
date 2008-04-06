//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/about_window.hpp>
#include <gui/util.hpp>
#include <util/version.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : About window

AboutWindow::AboutWindow(Window* parent)
	: wxDialog(parent, wxID_ANY, _TITLE_("about"), wxDefaultPosition, wxSize(510,340), wxCLIP_CHILDREN | wxDEFAULT_DIALOG_STYLE | wxTAB_TRAVERSAL)
	, logo (load_resource_image(_("about")))
	, logo2(load_resource_image(_("two_beta")))
{
	// init controls
	wxControl* ok_button = new HoverButton(this, wxID_OK, _("btn_ok"));
	wxSize bs = ok_button->GetSize(), ws = GetClientSize();
	ok_button->Move(ws.GetWidth() - bs.GetWidth(),  ws.GetHeight() - bs.GetHeight()); // align bottom right
}

void AboutWindow::onPaint(wxPaintEvent& ev) {
	wxBufferedPaintDC dc(this);
	draw(dc);
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
	dc.DrawText(_("Copyright \xA9 2007 Twan van Laarhoven"), 34, 214);
}

BEGIN_EVENT_TABLE(AboutWindow, wxDialog)
	EVT_PAINT      (AboutWindow::onPaint)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Button with image and hover effect


HoverButton::HoverButton(Window* parent, int id, const String& name, const Color& background, bool accepts_focus)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
	, hover(false), focus(false), mouse_down(false), key_down(false)
	, background(background)
	, accepts_focus(accepts_focus)
	, last_drawn(nullptr)
{
	loadBitmaps(name);
	SetSize(DoGetBestSize());
}

void HoverButton::loadBitmaps(const String& name) {
	if (bitmaps == name) return;
	bitmaps = name;
	bg_normal = Bitmap(load_resource_image(name + _("_normal")));
	bg_hover  = Bitmap(load_resource_image(name + _("_hover")));
	bg_focus  = Bitmap(load_resource_image(name + _("_focus")));
	bg_down   = Bitmap(load_resource_image(name + _("_down")));
	Refresh(false);
}

void HoverButton::onMouseEnter(wxMouseEvent&) {
	hover = true;
	refreshIfNeeded();
}
void HoverButton::onMouseLeave(wxMouseEvent&) {
	hover = false;
	refreshIfNeeded();
}
void HoverButton::onFocus(wxFocusEvent&) {
	focus = true;
	refreshIfNeeded();
}
void HoverButton::onKillFocus(wxFocusEvent&) {
	focus = false;
	refreshIfNeeded();
}
void HoverButton::onLeftDown(wxMouseEvent&) {
	mouse_down = true;
	SetFocus();
	CaptureMouse();
	refreshIfNeeded();
}
void HoverButton::onLeftUp(wxMouseEvent&) {
	if (HasCapture()) ReleaseMouse();
	if (mouse_down && hover) {
		mouse_down = false;
		refreshIfNeeded();
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		ProcessEvent(evt);
	}
	mouse_down = false;
}
void HoverButton::onKeyDown(wxKeyEvent& ev) {
	int code = ev.GetKeyCode();
	if (code == WXK_RETURN || code == WXK_SPACE) {
		key_down = true;
		refreshIfNeeded();
	} else {
		ev.Skip();
	}
}
void HoverButton::onKeyUp(wxKeyEvent& ev) {
	int code = ev.GetKeyCode();
	if (code == WXK_RETURN || code == WXK_SPACE) {
		key_down = false;
		refreshIfNeeded();
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		ProcessEvent(evt);
	}
}

wxSize HoverButton::DoGetBestSize() const {
	return wxSize(bg_normal.GetWidth(), bg_normal.GetHeight());
}
bool HoverButton::AcceptsFocus() const {
	return wxControl::AcceptsFocus() && accepts_focus;
}

const Bitmap* HoverButton::toDraw() const {
	return (mouse_down && hover) || key_down ? &bg_down
	     : hover                             ? &bg_hover
	     : focus                             ? &bg_focus
	     :                                     &bg_normal;
}
void HoverButton::refreshIfNeeded() {
	if (last_drawn != toDraw()) Refresh(false);
}

void HoverButton::onPaint(wxPaintEvent&) {
	wxPaintDC dc(this);
	draw(dc);
}
void HoverButton::draw(DC& dc) {
	// clear background (for transparent button images)
	wxSize ws = GetClientSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(background != wxNullColour ? background : wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	dc.DrawRectangle(0, 0, ws.GetWidth(), ws.GetHeight());
	// draw button
	dc.DrawBitmap(*toDraw(), 0, 0, true);
	last_drawn = toDraw();
}
int HoverButton::drawDelta() const {
	return (mouse_down && hover) || key_down ? 2 : 0;
}

BEGIN_EVENT_TABLE(HoverButton, wxControl)
	EVT_ENTER_WINDOW   (HoverButton::onMouseEnter)
	EVT_LEAVE_WINDOW   (HoverButton::onMouseLeave)
	EVT_PAINT          (HoverButton::onPaint)
	EVT_SET_FOCUS      (HoverButton::onFocus)
	EVT_KILL_FOCUS     (HoverButton::onKillFocus)
	EVT_LEFT_DOWN      (HoverButton::onLeftDown)
	EVT_LEFT_UP        (HoverButton::onLeftUp)
	EVT_KEY_DOWN       (HoverButton::onKeyDown)
	EVT_KEY_UP         (HoverButton::onKeyUp)
END_EVENT_TABLE  ()
