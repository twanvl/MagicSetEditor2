//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <algorithm>
#include "action_stack.hpp"
#include "for_each.hpp"

// ----------------------------------------------------------------------------- : Action stack

DECLARE_TYPEOF_COLLECTION(Action*);
DECLARE_TYPEOF_COLLECTION(ActionListener*);

ActionStack::ActionStack()
	: savePoint(nullptr)
{}

ActionStack::~ActionStack() {
	// we own the actions, delete them
	FOR_EACH(a, undoActions) delete a;
	FOR_EACH(a, redoActions) delete a;
}

void ActionStack::add(Action* action, bool allowMerge) {
	if (!action) return; // no action
	action->perform(false); // TODO: delete action if perform throws
	redoActions.clear();
	tellListeners(*action);
	// try to merge?
	if (allowMerge && !undoActions.empty() && undoActions.back()->merge(action)) {
		// merged with top undo action
		delete action;
	} else {
		undoActions.push_back(action);
	}
}

void ActionStack::undo() {
	assert(canUndo());
	Action* action = undoActions.back();
	action->perform(true);
	// move to redo stack
	undoActions.pop_back();
	redoActions.push_back(action);
}
void ActionStack::redo() {
	assert(canRedo());
	Action* action = redoActions.back();
	action->perform(false);
	// move to undo stack
	redoActions.pop_back();
	undoActions.push_back(action);
}

bool ActionStack::canUndo() const {
	return !undoActions.empty();
}
bool ActionStack::canRedo() const {
	return !redoActions.empty();
}

String ActionStack::undoName() const {
	if (canUndo()) {
		return _("Undo ") + capitalize(undoActions.back()->getName(true));
	} else {
		return _("Undo");
	}
}
String ActionStack::redoName() const {
	if (canRedo()) {
		return _("Redo ") + capitalize(redoActions.back()->getName(false));
	} else {
		return _("Redo");
	}
}

bool ActionStack::atSavePoint() const {
	return (undoActions.empty() && savePoint == nullptr)
	    || (undoActions.back() == savePoint);
}
void ActionStack::setSavePoint() {
	if (undoActions.empty()) {
		savePoint = nullptr;
	} else {
		savePoint = undoActions.back();
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
void ActionStack::tellListeners(const Action& action) {
	FOR_EACH(l, listeners) l->onAction(action);
}
