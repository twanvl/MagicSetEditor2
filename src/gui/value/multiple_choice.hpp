//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_MULTIPLE_CHOICE
#define HEADER_GUI_VALUE_MULTIPLE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/multiple_choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceValueEditor

/// An editor 'control' for editing MultipleChoiceValues
class MultipleChoiceValueEditor : public MultipleChoiceValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(MultipleChoice);
	
	virtual void determineSize(bool force_fit);
	virtual bool onLeftDown   (const RealPoint& pos, wxMouseEvent& ev);
  private:
	/// Toggle a choice or on or off
	void toggle(int id);
};

// ----------------------------------------------------------------------------- : EOF
#endif
