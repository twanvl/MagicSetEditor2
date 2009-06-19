//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_COLOR
#define HEADER_GUI_VALUE_COLOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/color.hpp>

DECLARE_SHARED_POINTER_TYPE(DropDownList);

// ----------------------------------------------------------------------------- : ColorValueEditor

/// An editor 'control' for editing ColorValues
class ColorValueEditor : public ColorValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Color);
	
	// --------------------------------------------------- : Events
	virtual bool onLeftDown(const RealPoint& pos, wxMouseEvent& ev);
	virtual bool onChar(wxKeyEvent& ev);
	virtual void onLoseFocus();
	
	virtual void draw(RotatedDC& dc);
	virtual void determineSize(bool);
	
  private:
	DropDownListP drop_down;
	friend class DropDownColorList;
	/// Change the color
	void change(const Defaultable<Color>& c);
	/// Change to a custom color
	void changeCustom();
};

// ----------------------------------------------------------------------------- : EOF
#endif
