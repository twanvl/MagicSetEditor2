//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	
	virtual void onLeftDClick(const RealPoint& pos, wxMouseEvent&);
	virtual void determineSize();
};

// ----------------------------------------------------------------------------- : EOF
#endif
