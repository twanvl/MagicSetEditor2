//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/card_select_window.hpp>

class PackageList;
class ExportOptionsEditor;
DECLARE_POINTER_TYPE(Set);

// ----------------------------------------------------------------------------- : HtmlExportWindow

class HtmlExportWindow : public ExportWindowBase {
public:
  HtmlExportWindow(Window* parent, const SetP& set, const ExportCardSelectionChoices& cards_choices);
  
private:
  PackageList*         list;    ///< List of templates
  ExportOptionsEditor* options; ///< Editor for template options
  SetP                 set;     ///< Set to export
  
  DECLARE_EVENT_TABLE();
  
  void onOk(wxCommandEvent&);
  void onTemplateSelect(wxCommandEvent&);
  void onUpdateUI(wxUpdateUIEvent& ev);
};

