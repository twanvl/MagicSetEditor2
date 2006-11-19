//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_CHOICE
#define HEADER_GUI_VALUE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/choice.hpp>

// ----------------------------------------------------------------------------- : ChoiceValueEditor

/// An editor 'control' for editing ChoiceValues
class ChoiceValueEditor : public ChoiceValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Choice);
};

// ----------------------------------------------------------------------------- : EOF
#endif
