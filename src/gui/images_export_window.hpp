//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/card_select_window.hpp>
#include <data/settings.hpp>

// ----------------------------------------------------------------------------- : ImagesExportWindow

/// A window for selecting a subset of the cards from a set to export to images
class ImagesExportWindow : public ExportWindowBase {
public:
  ImagesExportWindow(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices);
  
private:
  DECLARE_EVENT_TABLE();
  
  void onOk(wxCommandEvent&);
  
  wxTextCtrl* format;
  wxChoice*   conflicts;
};

