//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
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
  size_t selection() const override { return NO_SELECTION; }
  size_t itemCount() const override { return choices.size(); }
  String itemText(size_t item) const override { return choices.at(item); }
  void select(size_t item) override;
};

// ----------------------------------------------------------------------------- : FilterControl

FilterCtrl::FilterCtrl(wxWindow* parent, int id, String const& placeholder)
  : wxTextCtrl(parent, id, _(""), wxDefaultPosition, wxSize(160, -1), wxBORDER_THEME)
  , changing(false)
  , placeholder(placeholder)
{
  wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
  SetBackgroundColour(bg);
  SetCursor(wxCURSOR_IBEAM);
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

void FilterCtrl::focusAndSelect() {
  SetFocus();
  if (!value.empty()) {
    SetSelection(-1,-1);
  }
}

void FilterCtrl::update() {
  changing = true;
  if (!value.empty() || hasFocus()) {
    SetValue(value);
    wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    SetDefaultStyle(wxTextAttr(fg));
    SetForegroundColour(fg);
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    SetFont(font);
  } else {
    SetValue(placeholder);
    wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    SetDefaultStyle(wxTextAttr(lerp(fg,bg,0.5)));
    SetForegroundColour(lerp(fg,bg,0.5));
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.SetStyle(wxFONTSTYLE_ITALIC);
    SetFont(font);
  }
  clear_button->Show(!value.empty());
  changing = false;
}

void FilterCtrl::onChangeEvent(wxCommandEvent&) {
  if (!changing) {
    setFilter(GetValue(),true);
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
  wxSize cs = clear_button->GetBestSize();
  int margin = 2;
  clear_button->SetSize(s.x - cs.x - margin, (s.y-cs.y)/2, cs.x, cs.y);
}

void FilterCtrl::onSetFocus(wxFocusEvent& ev) {
  update();
  ev.Skip();
}
void FilterCtrl::onKillFocus(wxFocusEvent& ev) {
  update();
  ev.Skip();
}

bool FilterCtrl::hasFocus() {
  wxWindow* focus = wxWindow::FindFocus();
  return focus == this || focus == clear_button;
}

BEGIN_EVENT_TABLE(FilterCtrl, wxControl)
  EVT_BUTTON    (wxID_ANY, FilterCtrl::onClear)
  EVT_TEXT      (wxID_ANY, FilterCtrl::onChangeEvent)
  EVT_SIZE      (FilterCtrl::onSizeEvent)
  EVT_SET_FOCUS (FilterCtrl::onSetFocus)
  EVT_KILL_FOCUS(FilterCtrl::onKillFocus)
  EVT_CHAR      (FilterCtrl::onChar)
END_EVENT_TABLE()
