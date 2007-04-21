//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_INFORMATION
#define HEADER_GUI_VALUE_INFORMATION

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/information.hpp>

// ----------------------------------------------------------------------------- : InfoValueEditor

/// An editor 'control' for editing InfoValues
class InfoValueEditor : public InfoValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Info);
	
	virtual void determineSize(bool);
	virtual bool drawLabel() const { return false; }
};

// ----------------------------------------------------------------------------- : EOF
#endif
