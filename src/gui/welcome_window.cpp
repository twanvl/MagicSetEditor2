//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/welcome_window.hpp>
#include <gui/util.hpp>
#include <gui/new_window.hpp>
#include <gui/set/window.hpp>
#include <util/window_id.hpp>
#include <data/settings.hpp>
#include <data/format/formats.hpp>
#include <wx/dcbuffer.h>
#include <wx/filename.h>

// 2007-02-06: New HoverButton, hopefully this on works on GTK
#define USE_HOVERBUTTON

// ----------------------------------------------------------------------------- : WelcomeWindow

WelcomeWindow::WelcomeWindow()
	: Frame(nullptr, wxID_ANY, _TITLE_("magic set editor"), wxDefaultPosition, wxSize(480,340), wxDEFAULT_DIALOG_STYLE | wxTAB_TRAVERSAL | wxCLIP_CHILDREN )
	, logo (load_resource_image(_("about")))
	, logo2(load_resource_image(_("two")))
{
	SetIcon(load_resource_icon(_("app")));
	
	// init controls
	#ifdef USE_HOVERBUTTON
		wxControl* new_set   = new HoverButtonExt(this, ID_FILE_NEW,    load_resource_image(_("welcome_new")),  _BUTTON_("new set"),  _HELP_("new set"));	
		wxControl* open_set  = new HoverButtonExt(this, ID_FILE_OPEN,   load_resource_image(_("welcome_open")), _BUTTON_("open set"), _HELP_("open set"));
	#else
		wxControl* new_set   = new wxButton(this, ID_FILE_NEW,  _BUTTON_("new set"));
		wxControl* open_set  = new wxButton(this, ID_FILE_OPEN, _BUTTON_("open set"));
	#endif
	wxControl* open_last = nullptr;
	if (!settings.recent_sets.empty()) {
		wxFileName n(settings.recent_sets.front());
		#ifdef USE_HOVERBUTTON
			open_last       = new HoverButtonExt(this, ID_FILE_RECENT, load_resource_image(_("welcome_last")), _BUTTON_("last opened set"), _("Open '") + n.GetName() + _("'"));
		#else
			open_last       = new wxButton(this, ID_FILE_RECENT, _BUTTON_("last opened set"));
		#endif
	}
	
	wxSizer* s1  = new wxBoxSizer(wxHORIZONTAL);
	s1->AddSpacer(25);
		wxSizer* s2  = new wxBoxSizer(wxVERTICAL);
		s2->AddSpacer(100);
		s2->Add(new_set,   0, wxALL, 2);
		s2->Add(open_set,  0, wxALL, 2);
		if (open_last) s2->Add(open_last, 0, wxALL, 2);
		s2->AddStretchSpacer();
	s1->Add(s2);
	SetSizer(s1);
}

void WelcomeWindow::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}
void WelcomeWindow::draw(DC& dc) {
	wxSize ws = GetClientSize();
	// draw background
	dc.SetPen  (*wxTRANSPARENT_PEN);
	dc.SetBrush(Color(240,247,255));
	dc.DrawRectangle(0, 0, ws.GetWidth(), ws.GetHeight());
	// draw logo
	dc.DrawBitmap(logo,  (ws.GetWidth() -  logo.GetWidth()) / 2, 5);
	dc.DrawBitmap(logo2,  ws.GetWidth() - logo2.GetWidth(),      ws.GetHeight() - logo2.GetHeight());
}

void WelcomeWindow::onOpenSet(wxCommandEvent&) {
	wxFileDialog dlg(this, _TITLE_("open set"), wxEmptyString, wxEmptyString, import_formats(), wxOPEN);
	if (dlg.ShowModal() == wxID_OK) {
		wxBusyCursor wait;
		close(import_set(dlg.GetPath()));
	}
}

void WelcomeWindow::onNewSet(wxCommandEvent&) {
	close(new_set_window(this));
}

// TODO: MOVEME
template <typename T>
shared_ptr<T> open_package(const String& filename) {
	shared_ptr<T> package(new T);
	package->open(filename);
	return package;
}

void WelcomeWindow::onOpenLast(wxCommandEvent&) {
	wxBusyCursor wait;
	assert(!settings.recent_sets.empty());
	close( open_package<Set>(settings.recent_sets.front()) );
}

void WelcomeWindow::close(const SetP& set) {
	if (!set) return;
	(new SetWindow(nullptr, set))->Show();
	Close();
}


BEGIN_EVENT_TABLE(WelcomeWindow, wxFrame)
	EVT_BUTTON         (ID_FILE_NEW,    WelcomeWindow::onNewSet)
	EVT_BUTTON         (ID_FILE_OPEN,   WelcomeWindow::onOpenSet)
	EVT_BUTTON         (ID_FILE_RECENT, WelcomeWindow::onOpenLast)
	EVT_PAINT          (                WelcomeWindow::onPaint)
//	EVT_IDLE           (                WelcomeWindow::onIdle)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Hover button with label

HoverButtonExt::HoverButtonExt(Window* parent, int id, const wxImage& icon, const String& label, const String& sub_label)
	: HoverButton(parent, id, _("BTN"))
	, icon(icon)
	, label(label), sub_label(sub_label)
	, font_large(14, wxSWISS, wxNORMAL, wxNORMAL, false, _("Arial"))
	, font_small(8,  wxSWISS, wxNORMAL, wxNORMAL, false, _("Arial"))
{}

void HoverButtonExt::draw(DC& dc) {
	// draw button
	HoverButton::draw(dc);
	int d = drawDelta();
	// icon
	if (icon.Ok()) dc.DrawBitmap(icon, d+7, d+7);
	// text
	dc.SetTextForeground(*wxBLACK);
	dc.SetFont(font_large);
	dc.DrawText(label, d+44, d+7);
	dc.SetFont(font_small);
	dc.DrawText(sub_label, d+45, d+28);
}
