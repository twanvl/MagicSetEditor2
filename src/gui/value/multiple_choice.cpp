//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/multiple_choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceValueEditor

IMPLEMENT_VALUE_EDITOR(MultipleChoice) {}

void MultipleChoiceValueEditor::determineSize(bool force_fit) {
	if (!nativeLook()) return;
	// height depends on number of items and item height
	int item_count = field().choices->lastId();
	style().height = item_count * 20;
}
