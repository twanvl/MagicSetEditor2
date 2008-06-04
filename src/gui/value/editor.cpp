//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <data/action/value.hpp>

// ----------------------------------------------------------------------------- : ValueEditor

void ValueEditor::addAction(ValueAction* a) {
	a->isOnCard(editor().getCard().get());
	editor().addAction(a);
}
