//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_HTML_EXPORT_WINDOW
#define HEADER_GUI_HTML_EXPORT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/export_template.hpp>

class PackageList;
DECLARE_POINTER_TYPE(Set);

// ----------------------------------------------------------------------------- : HtmlExportWindow

class HtmlExportWindow : public wxDialog {
  public:
	HtmlExportWindow(Window* parent, const SetP& set);
	
  private:
	PackageList* list; ///< List of templates
	
	DECLARE_EVENT_TABLE();
	
	void onOk(wxCommandEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
