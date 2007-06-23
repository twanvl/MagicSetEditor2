//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_HTML_EXPORT_WINDOW
#define HEADER_GUI_HTML_EXPORT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class PackageList;
class ExportOptionsEditor;
DECLARE_POINTER_TYPE(Set);

// ----------------------------------------------------------------------------- : HtmlExportWindow

class HtmlExportWindow : public wxDialog {
  public:
	HtmlExportWindow(Window* parent, const SetP& set);
	
  private:
	PackageList*         list;    ///< List of templates
	ExportOptionsEditor* options; ///< Editor for template options
	SetP                 set;     ///< Set to export
	
	DECLARE_EVENT_TABLE();
	
	void onOk(wxCommandEvent&);
	void onTemplateSelect(wxCommandEvent&);
	void onUpdateUI(wxUpdateUIEvent& ev);
};

// ----------------------------------------------------------------------------- : EOF
#endif
