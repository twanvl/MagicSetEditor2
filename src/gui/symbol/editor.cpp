//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/editor.hpp>
#include <gui/symbol/window.hpp>

// ----------------------------------------------------------------------------- : SymbolEditorBase

void SymbolEditorBase::SetStatusText(const String& text) {
	control.parent->SetStatusText(text);
}
