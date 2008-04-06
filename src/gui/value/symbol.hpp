//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_SYMBOL
#define HEADER_GUI_VALUE_SYMBOL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/symbol.hpp>

// ----------------------------------------------------------------------------- : SymbolValueEditor

/// An editor 'control' for editing SymbolValues
class SymbolValueEditor : public SymbolValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Symbol);
	
	virtual void draw(RotatedDC& dc);
	virtual bool onLeftDown  (const RealPoint& pos, wxMouseEvent&);
	virtual bool onLeftUp    (const RealPoint& pos, wxMouseEvent&);
	virtual bool onLeftDClick(const RealPoint& pos, wxMouseEvent&);
	virtual bool onMotion    (const RealPoint& pos, wxMouseEvent&);
	virtual void determineSize(bool);
  private:
	/// Draw a button, buttons are numbered from the right
	void drawButton(RotatedDC& dc, int button, const String& text);
	/// Is there a button at the given position? returns the button index, or -1 if there is no button
	int findButton(const RealPoint& pos);
	
	// button, or -1 for mouse down, but not on button, or -2 for mouse not down
	int button_down;
	Bitmap button_images[1];
};

// ----------------------------------------------------------------------------- : EOF
#endif
