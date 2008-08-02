//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_IMAGES_EXPORT_WINDOW
#define HEADER_GUI_IMAGES_EXPORT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/card_select_window.hpp>
#include <data/settings.hpp>
class wxFileName;

// ----------------------------------------------------------------------------- : ImagesExportWindow

/// Export the cards in a set
class ExportCardImages {
  public:
	void exportImages(const SetP& set, wxFileName& filename, const String& filename_template, FilenameConflicts conflicts);
	virtual ~ExportCardImages() {}
  protected:
	/// Should the given card be exported?
	virtual bool includeCard(const CardP& card) const { return true; }
};

// ----------------------------------------------------------------------------- : ImagesExportWindow

/// A window for selecting a subset of the cards from a set to export to images
class ImagesExportWindow : public CardSelectWindow, private ExportCardImages {
  public:
	ImagesExportWindow(Window* parent, const SetP& set);
	
  private:
	DECLARE_EVENT_TABLE();
	
	void onOk(wxCommandEvent&);
	
	virtual bool includeCard(const CardP& card) const;
	
	wxTextCtrl* format;
	wxChoice*   conflicts;
};

// ----------------------------------------------------------------------------- : EOF
#endif
