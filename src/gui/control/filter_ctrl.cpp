//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/filter_ctrl.hpp>
#include <gui/about_window.hpp> // for HoverButton
#include <gui/drop_down_list.hpp>

// ----------------------------------------------------------------------------- : DropDownMRUList

/// A drop down list of recent choices, for autocomplete
class DropDownMRUList : public DropDownList {
  public:
	DropDownMRUList(Window* parent, vector<String> const& choices)
		: DropDownList(parent)
		, choices(choices)
	{}
	
	vector<String> choices;
	
  protected:
	virtual size_t selection() const { return NO_SELECTION; }
	virtual size_t itemCount() const { return choices.size(); }
	virtual String itemText(size_t item) const { return choices.at(item); }
	virtual void select(size_t item);
};

// ----------------------------------------------------------------------------- : FilterControl

/// Text control that forwards focus events to the parent
class TextCtrlWithFocus : public wxTextCtrl {
  public:
	DECLARE_EVENT_TABLE();
	void forwardFocusEvent(wxFocusEvent&);
	void forwardKeyEvent(wxKeyEvent&);
};

FilterCtrl::FilterCtrl(wxWindow* parent, int id, String const& placeholder)
	: wxControl(parent, id, wxDefaultPosition, wxSize(160,41), wxSTATIC_BORDER)
	, changing(false)
	, placeholder(placeholder)
{
	wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	SetBackgroundColour(bg);
	SetCursor(wxCURSOR_IBEAM);
	filter_ctrl = new TextCtrlWithFocus();
	filter_ctrl->Create(this, wxID_ANY, _(""), wxDefaultPosition, wxSize(130,-1), wxNO_BORDER);
	clear_button = new HoverButton(this, wxID_ANY, _("btn_clear_filter"), bg, false);
	clear_button->SetCursor(*wxSTANDARD_CURSOR);
	onSize();
	update();
}

void FilterCtrl::setFilter(const String& new_value, bool event) {
	if (this->value == new_value) return;
	// update ui
	this->value = new_value;
	update();
	// send event
	if (event) {
		wxCommandEvent ev(wxEVT_COMMAND_TEXT_UPDATED, GetId());
		GetParent()->HandleWindowEvent(ev);
	}
}

void FilterCtrl::update() {
	changing = true;
	if (!value.empty() || hasFocus()) {
		filter_ctrl->SetValue(value);
		wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
		filter_ctrl->SetDefaultStyle(wxTextAttr(fg));
		filter_ctrl->SetForegroundColour(fg);
		wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
		filter_ctrl->SetFont(font);
	} else {
		filter_ctrl->SetValue(placeholder);
		wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
		wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
		filter_ctrl->SetDefaultStyle(wxTextAttr(lerp(fg,bg,0.5)));
		filter_ctrl->SetForegroundColour(lerp(fg,bg,0.5));
		wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
		font.SetStyle(wxFONTSTYLE_ITALIC);
		filter_ctrl->SetFont(font);
	}
	clear_button->Show(!value.empty());
	changing = false;
}

void FilterCtrl::onChangeEvent(wxCommandEvent&) {
	if (!changing) {
		setFilter(filter_ctrl->GetValue(),true);
	}
}
void FilterCtrl::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_ESCAPE) {
		// escape clears the filter box
		clearFilter(true);
	} else {
		ev.Skip();
	}
}

void FilterCtrl::onClear(wxCommandEvent&) {
	clearFilter(true);
}

void FilterCtrl::onSizeEvent(wxSizeEvent&) {
	onSize();
}
void FilterCtrl::onSize() {
	wxSize s = GetClientSize();
	wxSize fs = filter_ctrl->GetBestSize();
	wxSize cs = clear_button->GetBestSize();
	int margin = 2;
	filter_ctrl ->SetSize(margin, max(margin,(s.y-fs.y)/2), s.x - cs.x - 3*margin, fs.y);
	clear_button->SetSize(s.x - cs.x - margin, (s.y-cs.y)/2, cs.x, cs.y);
}

void FilterCtrl::onSetFocus(wxFocusEvent&) {
	filter_ctrl->SetFocus();
	update();
}
void FilterCtrl::onKillFocus(wxFocusEvent&) {
	update();
}

bool FilterCtrl::hasFocus() {
	wxWindow* focus = wxWindow::FindFocus();
	return focus == this || focus == filter_ctrl || focus == clear_button;
}

BEGIN_EVENT_TABLE(FilterCtrl, wxControl)
	EVT_BUTTON    (wxID_ANY, FilterCtrl::onClear)
	EVT_TEXT      (wxID_ANY, FilterCtrl::onChangeEvent)
	EVT_SIZE      (FilterCtrl::onSizeEvent)
	EVT_SET_FOCUS (FilterCtrl::onSetFocus)
	EVT_KILL_FOCUS(FilterCtrl::onKillFocus)
	EVT_CHAR      (FilterCtrl::onChar)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : TextCtrlWithFocus

void TextCtrlWithFocus::forwardFocusEvent(wxFocusEvent& ev) {
	GetParent()->HandleWindowEvent(ev);
}
void TextCtrlWithFocus::forwardKeyEvent(wxKeyEvent& ev) {
	GetParent()->HandleWindowEvent(ev);
}

BEGIN_EVENT_TABLE(TextCtrlWithFocus, wxTextCtrl)
	EVT_SET_FOCUS (TextCtrlWithFocus::forwardFocusEvent)
	EVT_KILL_FOCUS(TextCtrlWithFocus::forwardFocusEvent)
	EVT_CHAR      (TextCtrlWithFocus::forwardKeyEvent)
END_EVENT_TABLE()

