//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>
#include <gui/symbol/window.hpp>

// ----------------------------------------------------------------------------- : SymbolEditorBase

void SymbolEditorBase::SetStatusText(const String& text) {
	control.parent->SetStatusText(text);
}

void SymbolEditorBase::addAction(Action* action, bool allow_merge) {
	getSymbol()->actions.addAction(action, allow_merge);
}
