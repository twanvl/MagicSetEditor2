//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_CARD_VIEWER
#define HEADER_GUI_CONTROL_CARD_VIEWER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : CardViewer

/// A control to view a single card
class CardViewer : public wxControl, public DataViewer {
  public:
	CardViewer(Window* parent, int id, long style = 0);
	
  protected:
	/// Return the desired size of control
	virtual wxSize DoGetBestSize() const;
	
	virtual void onChange();
	
  private:
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
	
	Bitmap buffer; /// < Off-screen buffer we draw to
};

// ----------------------------------------------------------------------------- : EOF
#endif
