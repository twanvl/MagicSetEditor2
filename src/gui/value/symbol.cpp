//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/symbol.hpp>
#include <gui/symbol/window.hpp>

// ----------------------------------------------------------------------------- : SymbolValueEditor

IMPLEMENT_VALUE_EDITOR(Symbol) {}

bool SymbolValueEditor::onLeftDClick(const RealPoint& pos, wxMouseEvent&) {
	// TODO : use SetWindow as parent? Maybe not, the symbol editor will stay open when mainwindow closes
	SymbolWindow* wnd = new SymbolWindow(nullptr, valueP(), viewer.getSet());
	wnd->Show();
	return true;
}

void SymbolValueEditor::determineSize(bool) {
	style().height = 50;
}
