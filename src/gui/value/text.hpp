//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_TEXT
#define HEADER_GUI_VALUE_TEXT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/text.hpp>

// ----------------------------------------------------------------------------- : TextValueEditor

/// An editor 'control' for editing TextValues
class TextValueEditor : public TextValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Text);
	
//	virtual void determineSize();
};

// ----------------------------------------------------------------------------- : EOF
#endif
