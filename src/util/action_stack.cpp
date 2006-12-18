//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/action_stack.hpp>
#include <util/for_each.hpp>
#include <algorithm>

// ----------------------------------------------------------------------------- : Action stack

DECLARE_TYPEOF_COLLECTION(Action*);
DECLARE_TYPEOF_COLLECTION(ActionListener*);

ActionStack::ActionStack()
	: save_point(nullptr)
{}

ActionStack::~ActionStack() {
	// we own the actions, delete them
	FOR_EACH(a, undo_actions) delete a;
	FOR_EACH(a, redo_actions) delete a;
}

void ActionStack::add(Action* action, bool allow_merge) {
	if (!action) return; // no action
	action->perform(false); // TODO: delete action if perform throws
	tellListeners(*action, false);
	// clear redo list
	FOR_EACH(a, redo_actions) delete a;
	redo_actions.clear();
	// try to merge?
	if (allow_merge && !undo_actions.empty() && undo_actions.back()->merge(*action)) {
		// merged with top undo action
		delete action;
	} else {
		undo_actions.push_back(action);
	}
}

void ActionStack::undo() {
	assert(canUndo());
	if (!canUndo()) return;
	Action* action = undo_actions.back();
	action->perform(true);
	tellListeners(*action, true);
	// move to redo stack
	undo_actions.pop_back();
	redo_actions.push_back(action);
}
void ActionStack::redo() {
	assert(canRedo());
	if (!canRedo()) return;
	Action* action = redo_actions.back();
	action->perform(false);
	tellListeners(*action, false);
	// move to undo stack
	redo_actions.pop_back();
	undo_actions.push_back(action);
}

bool ActionStack::canUndo() const {
	return !undo_actions.empty();
}
bool ActionStack::canRedo() const {
	return !redo_actions.empty();
}

String ActionStack::undoName() const {
	if (canUndo()) {
		return _(" ") + capitalize(undo_actions.back()->getName(true));
	} else {
		return wxEmptyString;
	}
}
String ActionStack::redoName() const {
	if (canRedo()) {
		return _(" ") + capitalize(redo_actions.back()->getName(false));
	} else {
		return wxEmptyString;
	}
}

bool ActionStack::atSavePoint() const {
	return (undo_actions.empty() && save_point == nullptr)
	    || (undo_actions.back() == save_point);
}
void ActionStack::setSavePoint() {
	if (undo_actions.empty()) {
		save_point = nullptr;
	} else {
		save_point = undo_actions.back();
	}
}

void ActionStack::addListener(ActionListener* listener) {
	listeners.push_back(listener);
}
void ActionStack::removeListener(ActionListener* listener) {
	listeners.erase(
		std::remove(
			listeners.begin(),
			listeners.end(),
			listener
			),
		listeners.end()
		);
}
void ActionStack::tellListeners(const Action& action, bool undone) {
	FOR_EACH(l, listeners) l->onAction(action, undone);
}
