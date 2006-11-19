//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_SYMBOL
#define HEADER_RENDER_VALUE_SYMBOL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/symbol.hpp>

// ----------------------------------------------------------------------------- : SymbolValueViewer

/// Viewer that displays a symbol value
class SymbolValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(Symbol) : ValueViewer(parent,style) {}
	
	virtual void draw(RotatedDC& dc);
};

// ----------------------------------------------------------------------------- : EOF
#endif
