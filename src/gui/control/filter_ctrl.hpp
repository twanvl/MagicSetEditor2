//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_FILTER_CTRL
#define HEADER_GUI_CONTROL_FILTER_CTRL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/filter.hpp>

class HoverButton;
class TextCtrlWithFocus;

// ----------------------------------------------------------------------------- : FilterCtrl

/// A search/filter textbox
class FilterCtrl : public wxControl {
  public:
	FilterCtrl(wxWindow* parent, int id, String const& placeholder);
	
	/// Set the filter text
	void setFilter(const String& filter, bool send_event = false);
	void clearFilter(bool send_event = false) { setFilter(String(),send_event); }
	bool hasFilter() const { return !value.empty(); }
	String const& getFilterString() const { return value; }
	
	template <typename T>
	intrusive_ptr<Filter<T> > getFilter() const {
		if (hasFilter()) {
			return intrusive(new QuickFilter<T>(getFilterString()));
		} else {
			return intrusive_ptr<Filter<T> >();
		}
	}
	
  private:
	DECLARE_EVENT_TABLE();
	bool changing;
	String value;
	String placeholder;
	TextCtrlWithFocus* filter_ctrl;
	HoverButton* clear_button;
	
	void update();
	bool hasFocus();
	// wxWidgets appears to have developed an overload allergy
	void onChangeEvent(wxCommandEvent&);
	void onClear(wxCommandEvent&);
	void onSizeEvent(wxSizeEvent&);
	void onChar(wxKeyEvent&);
	void onSize();
	void onSetFocus(wxFocusEvent&);
	void onKillFocus(wxFocusEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
