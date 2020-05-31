//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/information.hpp>

// ----------------------------------------------------------------------------- : InfoValueEditor

IMPLEMENT_VALUE_EDITOR(Info) {}

void InfoValueEditor::determineSize(bool) {
  bounding_box.height = 26;
}
