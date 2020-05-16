//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/filter.hpp>

class HoverButton;

// ----------------------------------------------------------------------------- : FilterCtrl

/// A search/filter textbox
class FilterCtrl : public wxTextCtrl {
public:
  FilterCtrl(wxWindow* parent, int id, String const& placeholder, String const& help_text);
  
  /// Set the filter text
  void setFilter(const String& filter, bool send_event = false);
  void clearFilter(bool send_event = false) { setFilter(String(),send_event); }
  bool hasFilter() const { return !value.empty(); }
  String const& getFilterString() const { return value; }
  void focusAndSelect();
  
  template <typename T>
  intrusive_ptr<Filter<T>> getFilter() const {
    if (hasFilter()) {
      return make_intrusive<QuickFilter<T>>(getFilterString());
    } else {
      return intrusive_ptr<Filter<T>>();
    }
  }
  
private:
  DECLARE_EVENT_TABLE();
  bool changing;
  String value;
  String placeholder;
  String help_text;
  HoverButton* clear_button;
  
  void update();
  bool hasFocus();
  void showHelp(bool show = true);
  // wxWidgets appears to have developed an overload allergy
  void onChangeEvent(wxCommandEvent&);
  void onClear(wxCommandEvent&);
  void onSizeEvent(wxSizeEvent&);
  void onChar(wxKeyEvent&);
  void onSize();
  void onSetFocus(wxFocusEvent&);
  void onKillFocus(wxFocusEvent&);
  void onMouseEnter(wxMouseEvent&);
  void onMouseLeave(wxMouseEvent&);
};

