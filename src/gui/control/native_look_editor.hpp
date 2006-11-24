//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_NATIVE_LOOK_EDITOR
#define HEADER_GUI_CONTROL_NATIVE_LOOK_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_editor.hpp>

// ----------------------------------------------------------------------------- : NativeLookEditor

/// A data editor with a platform native look
class NativeLookEditor : public DataEditor {
  public:
	NativeLookEditor(Window* parent, int id, long style = 0);
	
	/// Uses a native look
	virtual bool nativeLook()  const { return true; }
	virtual bool drawBorders() const { return false; }
	virtual Rotation getRotation() const;
	
	virtual void draw(DC& dc);
	virtual void drawViewer(RotatedDC& dc, ValueViewer& v);
	
  protected:
	// Best size doesn't really matter, as long as it is not too small
	virtual wxSize DoGetBestSize() const;
	virtual void onInit();
	
  private:
	static const UInt margin      = 6;
	static const UInt margin_left = 4;
	static const UInt label_width = 150;
	static const UInt vspace      = 10;
	
	DECLARE_EVENT_TABLE();
	
	void onSize(wxSizeEvent&);
	/// Resize the viewers so they match with this control
	void resizeViewers();
};


// ----------------------------------------------------------------------------- : SetInfoEditor

/// Editor for set.data
class SetInfoEditor : public NativeLookEditor {
  public:
	SetInfoEditor(Window* parent, int id, long style = 0);
  protected:
	virtual void onChangeSet();
};

// ----------------------------------------------------------------------------- : StylingEditor

/// Editor for styling data
class StylingEditor : public NativeLookEditor {
  public:
	StylingEditor(Window* parent, int id, long style = 0);
	
	/// Show the styling for given stylesheet in the editor	
	void showStylesheet(const StyleSheetP& stylesheet);
	
  protected:
	virtual void onChangeSet();
	
  private:
	StyleSheetP stylesheet; ///< The stylesheet for which we are showing the styling data
};

// ----------------------------------------------------------------------------- : EOF
#endif
