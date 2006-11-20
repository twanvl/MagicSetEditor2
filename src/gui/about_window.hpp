//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_ABOUT_WINDOW
#define HEADER_GUI_ABOUT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : About window

/// Nice about dialog
class AboutWindow : public wxDialog {
  public:
	AboutWindow(Window* parent);
	
  private:
	DECLARE_EVENT_TABLE();

	// graphics
	Bitmap logo, logo2;
	
	void onPaint(wxPaintEvent&);
	void draw(DC& dc);
};

// ----------------------------------------------------------------------------- : Button with image and hover effect

// A button that changes images on mouseenter/leave
class HoverButton : public wxBitmapButton {
  public:
	HoverButton(Window* parent, int id, const String& name);
	
  private:
	DECLARE_EVENT_TABLE();
	
	Bitmap normal, hover; /// Bitmaps for the states of the button
	
	void onMouseEnter(wxMouseEvent&);
	void onMouseLeave(wxMouseEvent&);
	void onFocus     (wxFocusEvent& ev);
	void onKillFocus (wxFocusEvent& ev);
	void onPaint     (wxPaintEvent&);
	
  protected:
	virtual void draw(DC& dc);
};


// ----------------------------------------------------------------------------- : EOF
#endif
