//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/multiple_choice.hpp>
#include <data/action/value.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceValueEditor

IMPLEMENT_VALUE_EDITOR(MultipleChoice) {}

void MultipleChoiceValueEditor::determineSize(bool force_fit) {
	if (!nativeLook()) return;
	// item height
	item_height = 16;
	// height depends on number of items and item height
	int item_count = field().choices->lastId();
	style().height = item_count * item_height;
}

bool MultipleChoiceValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	// find item under cursor
	if (style().render_style && RENDER_CHECKLIST) {
		int id = (pos.y - style().top) / item_height;
		int end = field().choices->lastId();
		if (id >= 0 && id < end) {
			toggle(id);
			return true;
		}
	} else {
		// TODO
	}
	return false;
}

void MultipleChoiceValueEditor::toggle(int id) {
	String new_value;
	// old selection
	vector<String> selected;
	value().get(selected);
	vector<String>::iterator select_it = selected.begin();
	// copy selected choices to new value
	int end = field().choices->lastId();
	for (int i = 0 ; i < end ; ++i) {
		String choice = field().choices->choiceName(i);
		bool active = select_it != selected.end() && *select_it == choice;
		if (active) select_it++;
		if (active != (i == id)) {
			if (!new_value.empty()) new_value += _(", ");
			new_value += choice;
		}
	}
	// store value
	getSet().actions.add(value_action(valueP(), new_value));
}
