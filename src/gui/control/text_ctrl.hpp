//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_TEXT_CTRL
#define HEADER_GUI_CONTROL_TEXT_CTRL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_editor.hpp>

// ----------------------------------------------------------------------------- : TextCtrl

/// A control for editing a String
/** Implemented using a DataEditor. A fake Field and Value are created for the item.
 *
 *  TODO:
 *  Possible problem: If multiple TextCtrls are editing the same value they will both
 *                    create a Value, actions modifying one of them will not match with actions modifying the other
 *  Possible solution: 1. Global map of Values
 *                     2. Ignore the problem, it will not happen in practice
 */
class TextCtrl : public DataEditor {
  public:
	TextCtrl(Window* parent, int id, long style = 0);
	
	/// Set the value that is being edited
	void setValue(String* value);
	/// Notification that the value has changed outside this control
	void valueChanged();
	
	/// Uses a native look
	virtual bool nativeLook()  const { return true; }
	virtual bool drawBorders() const { return false; }
	virtual Rotation getRotation() const;
	
	virtual void draw(DC& dc);
	
	/// When an action is received, change the underlying value
	virtual void onAction(const Action&, bool undone);
	virtual void onChangeSet();
	
  protected:
	virtual void onInit();
	virtual wxSize DoGetBestSize() const;
	
  private:
	String* value; ///< Value to edit
	
	DECLARE_EVENT_TABLE();
	
	void onSize(wxSizeEvent&);
};


// ----------------------------------------------------------------------------- : EOF
#endif
