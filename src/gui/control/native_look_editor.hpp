//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_NATIVE_LOOK_EDITOR
#define HEADER_GUI_CONTROL_NATIVE_LOOK_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_editor.hpp>

DECLARE_POINTER_TYPE(ExportTemplate);

// ----------------------------------------------------------------------------- : NativeLookEditor

/// A data editor with a platform native look
class NativeLookEditor : public DataEditor {
  public:
	NativeLookEditor(Window* parent, int id, long style = wxBORDER_THEME);
	
	/// Uses a native look
	virtual bool nativeLook()  const { return true; }
	virtual Rotation getRotation() const;
	
	virtual void draw(DC& dc);
	virtual void drawViewer(RotatedDC& dc, ValueViewer& v);
	
  protected:
	// Best size doesn't really matter, as long as it is not too small
	virtual wxSize DoGetBestSize() const;
	virtual void onInit();
	
  private:
	static const int margin      = 6;
	static const int margin_left = 4;
	static const int vspace      = 10;
	static const int label_margin = 10;
	int              label_width;
	
	DECLARE_EVENT_TABLE();
	
	void onSize(wxSizeEvent&);
	void onScroll(wxScrollWinEvent&);
	void onMouseWheel(wxMouseEvent&);
	void scrollTo(int direction, int pos);
	/// Resize the viewers so they match with this control
	void resizeViewers();
};


// ----------------------------------------------------------------------------- : SetInfoEditor

/// Editor for set.data
class SetInfoEditor : public NativeLookEditor {
  public:
	SetInfoEditor(Window* parent, int id, long style = wxBORDER_THEME);
	
	virtual Package& getStylePackage() const;
  protected:
	virtual void onChangeSet();
};

// ----------------------------------------------------------------------------- : StylingEditor

/// Editor for styling data
class StylingEditor : public NativeLookEditor {
  public:
	StylingEditor(Window* parent, int id, long style = wxBORDER_THEME);
	
	/// Show the styling for given stylesheet in the editor
	void showStylesheet(const StyleSheetP& stylesheet);
	/// Show the styling for given card
	void showCard(const CardP& card);
  protected:
	virtual void onChangeSet();
};

// ----------------------------------------------------------------------------- : ExportOptionsEditor

/// Editor for export options
class ExportOptionsEditor : public NativeLookEditor {
  public:
	ExportOptionsEditor(Window* parent, int id, long style = wxBORDER_THEME);
	
	/// Show the options for given export template
	void showExport(const ExportTemplateP& export_template);
	
	virtual Package& getStylePackage() const;
  private:
	ExportTemplateP export_template;
};

// ----------------------------------------------------------------------------- : EOF
#endif
