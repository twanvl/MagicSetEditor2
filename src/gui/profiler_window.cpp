//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/profiler.hpp>
#include <wx/dcbuffer.h>

#if USE_SCRIPT_PROFILING

DECLARE_TYPEOF_COLLECTION(FunctionProfileP);

// -----------------------------------------------------------------------------
// Profiler Panel : class
// -----------------------------------------------------------------------------

class ProfilerPanel : public wxPanel {
  public:
	ProfilerPanel(wxWindow* parent, bool fancy_effects);
	~ProfilerPanel();
	
	virtual bool AcceptsFocus() const { return false; }
	virtual wxSize DoGetBestSize() const;
	
  private:
	bool        fancy_effects;
	wxTimer     timer;
	wxStopWatch stopwatch;
	
	typedef std::pair<int,long> NumCallsAndFadeTime;
	typedef std::map<FunctionProfile const*,NumCallsAndFadeTime> PrevProfiles;
	PrevProfiles prev_profiles;
	
	DECLARE_EVENT_TABLE();
	void onPaint(wxPaintEvent&);
	void onEraseBackground(wxEraseEvent&) {}
	void onTimer(wxTimerEvent&);
	void onSize(wxSizeEvent&);
	void draw_profiler(wxDC& dc, int x, int y);
};

// -----------------------------------------------------------------------------
// Drawing utilities
// -----------------------------------------------------------------------------

void clear_dc(wxDC& dc, wxColour const& color) {
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(color);
	wxSize s = dc.GetSize();
	dc.DrawRectangle(0,0,s.x,s.y);
}

void draw_right(wxDC& dc, wxString const& text, int x, int y) {
	int w, h;
	dc.GetTextExtent(text, &w, &h);
	dc.DrawText(text, x-w, y);
}

// -----------------------------------------------------------------------------
// Profiler Panel
// -----------------------------------------------------------------------------

ProfilerPanel::ProfilerPanel(wxWindow* parent, bool fancy_effects)
	: wxPanel(parent, wxID_ANY)
	, fancy_effects(fancy_effects)
	, timer(this)
{
//	profiler::set_function_leave_callback(refresh_profiler_panel);
}

ProfilerPanel::~ProfilerPanel() {
//	profiler::set_function_leave_callback(nullptr);
}


void ProfilerPanel::onPaint(wxPaintEvent&) {
	#ifdef __WXMSW__
		wxBufferedPaintDC dc(this);
	#else
		wxPaintDC dc(this);
	#endif
	// clear background
	clear_dc(dc, wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	// draw table
	dc.SetFont(*wxNORMAL_FONT);
	draw_profiler(dc, 0, 0);
}

void ProfilerPanel::draw_profiler(wxDC& dc, int x0, int y0) {
	#if USE_SCRIPT_PROFILING
		// Get the profiles
		const FunctionProfile& profile = profile_aggregated(1);
		vector<FunctionProfileP> profiles;
		profile.get_children(profiles);
		// set up colors
		wxColour fg(0,0,0);
		wxColour fg_highlight(0,20,220);
		dc.SetTextForeground(fg);
		dc.SetPen(fg);
		// set up positions/sizes
		int line_height = dc.GetCharHeight() + 2;
		int x1 = dc.GetSize().x - 2;
		int pos[] = {x0+2, x1-124, x1-84, x1-44, x1-4 };
		// fancy effects
		bool any_active = false;
		long now = stopwatch.Time();
		// Draw table
		dc.DrawText(_("Function"), pos[0], y0 + 2);
		draw_right(dc,_("calls"),  pos[1], y0 + 2);
		draw_right(dc,_("avg"),    pos[2], y0 + 2);
		draw_right(dc,_("total"),  pos[3], y0 + 2);
		draw_right(dc,_("max"),    pos[4], y0 + 2);
		dc.DrawLine(x0, y0 + line_height + 2, x1, y0 + line_height + 2);
		int i = 0;
		FOR_EACH_REVERSE(prof, profiles) {
			// recently changed?
			if (fancy_effects) {
				PrevProfiles::iterator prev = prev_profiles.find(prof.get());
				float active = 0.f;
				const int FADE_TIME = 1000;
				if (prev == prev_profiles.end()) {
					prev_profiles[prof.get()] = std::make_pair(prof->calls,0);
				} else if (prev->second.first != prof->calls) {
					prev->second = std::make_pair(prof->calls,now+FADE_TIME);
					any_active = true;
					active = 1.f;
					dc.SetTextForeground(*wxRED);
				} else if (now < prev->second.second) {
					active = (prev->second.second - now) / (float)FADE_TIME;
					any_active = true;
				}
				dc.SetTextForeground(lerp(fg,fg_highlight,active));
			}
			// draw line
			int y = y0 + (++i) * line_height + 6;
			dc.DrawText(prof->name,                                        pos[0], y);
			draw_right(dc,wxString::Format(_("%d"),   prof->calls),        pos[1], y);
			draw_right(dc,wxString::Format(_("%.2f"), prof->avg_time()),   pos[2], y);
			draw_right(dc,wxString::Format(_("%.2f"), prof->total_time()), pos[3], y);
			draw_right(dc,wxString::Format(_("%.2f"), prof->max_time()),   pos[4], y);
		}
		// are any fancy effects active?
		if (fancy_effects && any_active && !timer.IsRunning()) {
			timer.Start(40,wxTIMER_ONE_SHOT);
		}
//%		profiler_panel_refreshing = false;
	#endif
}

void ProfilerPanel::onTimer(wxTimerEvent&) {
	Refresh(false);
}

void ProfilerPanel::onSize(wxSizeEvent&) {
	Refresh(true);
}

wxSize ProfilerPanel::DoGetBestSize() const {
	return wxSize(300,500);
}

BEGIN_EVENT_TABLE(ProfilerPanel, wxPanel)
	EVT_PAINT(          ProfilerPanel::onPaint)
	EVT_SIZE(           ProfilerPanel::onSize)
	EVT_TIMER(wxID_ANY, ProfilerPanel::onTimer)
	EVT_ERASE_BACKGROUND(ProfilerPanel::onEraseBackground)
END_EVENT_TABLE()


void show_profiler_window(wxWindow* parent) {
	wxDialog* dlg = new wxDialog(parent, wxID_ANY, _("Profiler"), wxDefaultPosition,wxSize(450,600), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new ProfilerPanel(dlg,true), 1, wxEXPAND | wxALL, 8);
	sizer->Add(dlg->CreateButtonSizer(wxOK), 0, wxALIGN_CENTER | wxALL, 8);
	dlg->SetSizer(sizer);
	dlg->Show();
}

#endif
