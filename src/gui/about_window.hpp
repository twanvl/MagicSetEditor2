//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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

// ----------------------------------------------------------------------------- : Button with hover effect

/// A button that changes images on mouseenter/leave
class HoverButtonBase : public wxControl {
  public:
	HoverButtonBase(Window* parent, int id, bool accepts_focus = true);
	
	virtual bool AcceptsFocus() const;
	
	virtual void SetHelpText(const String& s) { help_text = s; }
	
  private:
	DECLARE_EVENT_TABLE();
	
	const bool accepts_focus;
	
	void onMouseEnter(wxMouseEvent&);
	void onMouseLeave(wxMouseEvent&);
	void onFocus     (wxFocusEvent& ev);
	void onKillFocus (wxFocusEvent& ev);
	void onLeftUp    (wxMouseEvent&);
	void onLeftDown  (wxMouseEvent&);
	void onLoseCapture(wxMouseCaptureLostEvent&);
	void onKeyDown   (wxKeyEvent&);
	void onKeyUp     (wxKeyEvent&);
	void onPaint     (wxPaintEvent&);
	
  protected:
	bool hover, focus, mouse_down, key_down;
	String help_text;
	
	virtual void draw(DC& dc) = 0;
	virtual void refreshIfNeeded();
	virtual void onClick();
};

// ----------------------------------------------------------------------------- : Button with image and hover effect

/// A button that changes images on mouseenter/leave
class HoverButton : public HoverButtonBase {
  public:
	/// Create a HoverButton, name is the resource name of the images to use
	/** name+"_normal", name+"_hover", name+"_focus", name+"_down"
	 *  are the resource names of the images used.
	 */
	HoverButton(Window* parent, int id, const String& name, const Color& background = Color(240,247,255), bool accepts_focus = true);
	
	/// Load different bitmaps for this button
	void loadBitmaps(const String& name);
	
  private:
	String bitmaps; ///< Name of the loaded bitmaps
	Bitmap bg_normal, bg_hover, bg_focus, bg_down; ///< Bitmaps for the states of the button
	Color background;
	
	virtual wxSize DoGetBestSize() const;
	
	const Bitmap* last_drawn;
	const Bitmap* toDraw() const;
  protected:
	int drawDelta() const;
	virtual void refreshIfNeeded();
	virtual void draw(DC& dc);
};


// ----------------------------------------------------------------------------- : EOF
#endif
