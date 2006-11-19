//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_COLOR
#define HEADER_GUI_VALUE_COLOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/color.hpp>

// ----------------------------------------------------------------------------- : ColorValueEditor

/// An editor 'control' for editing ColorValues
class ColorValueEditor : public ColorValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Color);
};

// ----------------------------------------------------------------------------- : EOF
#endif
