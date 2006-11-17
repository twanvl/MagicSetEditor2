//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/symbol_font.hpp>

// ----------------------------------------------------------------------------- : SymbolFont
// ----------------------------------------------------------------------------- : SymbolFontRef

SymbolFontRef::SymbolFontRef()
	: size(12)
	, scale_down_to(1)
	, alignment(ALIGN_MIDDLE_CENTER)
{}

IMPLEMENT_REFLECTION(SymbolFontRef) {
	REFLECT(name);
	REFLECT(size);
	REFLECT(scale_down_to);
	REFLECT(alignment);
}
