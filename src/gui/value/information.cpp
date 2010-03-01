//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/information.hpp>

// ----------------------------------------------------------------------------- : InfoValueEditor

IMPLEMENT_VALUE_EDITOR(Info) {}

void InfoValueEditor::determineSize(bool) {
	style().height = 26;
}
