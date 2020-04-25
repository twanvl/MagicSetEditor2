//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <data/action/value.hpp>

// ----------------------------------------------------------------------------- : ValueEditor

void ValueEditor::addAction(unique_ptr<ValueAction> a) {
  if (a) {
    a->isOnCard(editor().getCard().get());
    editor().addAction(move(a));
  }
}
