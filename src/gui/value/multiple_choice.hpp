//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_MULTIPLE_CHOICE
#define HEADER_GUI_VALUE_MULTIPLE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <gui/value/choice.hpp>
#include <render/value/multiple_choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceValueEditor

/// An editor 'control' for editing MultipleChoiceValues
class MultipleChoiceValueEditor : public MultipleChoiceValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(MultipleChoice);
	~MultipleChoiceValueEditor();
	
	virtual void onValueChange();
	
	virtual void determineSize(bool force_fit);
	
	virtual bool onLeftDown   (const RealPoint& pos, wxMouseEvent& ev);
	virtual bool onChar(wxKeyEvent& ev);
	virtual void onLoseFocus();
	
  private:
	DropDownListP drop_down;
	vector<int> active;      ///< Which choices are active? (note: vector<bool> is evil)
	friend class DropDownMultipleChoiceList;
	/// Initialize the drop down list
	DropDownList& initDropDown();
	/// Toggle a choice or on or off
	void toggle(int id);
	/// Toggle defaultness or on or off
	void toggleDefault();
};

// ----------------------------------------------------------------------------- : EOF
#endif
