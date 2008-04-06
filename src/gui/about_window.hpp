//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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

/// A button that changes images on mouseenter/leave
class HoverButton : public wxControl {
  public:
	/// Create a HoverButton, name is the resource name of the images to use
	/** name+"_normal", name+"_hover", name+"_focus", name+"_down"
	 *  are the resource names of the images used.
	 */
	HoverButton(Window* parent, int id, const String& name, const Color& background = Color(240,247,255), bool accepts_focus = true);
	
	/// Load different bitmaps for this button
	void loadBitmaps(const String& name);
	
	virtual bool AcceptsFocus() const;
	
  private:
	DECLARE_EVENT_TABLE();
	
	String bitmaps; ///< Name of the loaded bitmaps
	Bitmap bg_normal, bg_hover, bg_focus, bg_down; ///< Bitmaps for the states of the button
	bool hover, focus, mouse_down, key_down;
	Color background;
	const bool accepts_focus;
	
	void onMouseEnter(wxMouseEvent&);
	void onMouseLeave(wxMouseEvent&);
	void onFocus     (wxFocusEvent& ev);
	void onKillFocus (wxFocusEvent& ev);
	void onPaint     (wxPaintEvent&);
	void onLeftUp    (wxMouseEvent&);
	void onLeftDown  (wxMouseEvent&);
	void onKeyDown   (wxKeyEvent&);
	void onKeyUp     (wxKeyEvent&);
	virtual wxSize DoGetBestSize() const;
	
	const Bitmap* last_drawn;
	const Bitmap* toDraw() const;
	void refreshIfNeeded();
	
  protected:
	virtual void draw(DC& dc);
	int drawDelta() const;
};


// ----------------------------------------------------------------------------- : EOF
#endif
