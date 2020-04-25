//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2017 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <util/for_each.hpp>
#include <algorithm>

// ----------------------------------------------------------------------------- : Action stack

ActionStack::ActionStack()
  : save_point(nullptr)
  , last_was_add(false)
{}

void ActionStack::addAction(unique_ptr<Action> action, bool allow_merge) {
  if (!action) return; // no action
  action->perform(false); // TODO: delete action if perform throws
  tellListeners(*action, false);
  // clear redo list
  if (!redo_actions.empty()) allow_merge = false; // don't merge after undo
  redo_actions.clear();
  // try to merge?
  if (allow_merge && !undo_actions.empty() &&
      last_was_add                            && // never merge with something that was redone once already
      undo_actions.back().get() != save_point && // never merge with the save point
      undo_actions.back()->merge(*action) // merged with top undo action
      ) {
    // don't add
  } else {
    undo_actions.push_back(move(action));
  }
  last_was_add = true;
}

void ActionStack::undo() {
  assert(canUndo());
  if (!canUndo()) return;
  unique_ptr<Action> action = move(undo_actions.back());
  undo_actions.pop_back();
  action->perform(true);
  tellListeners(*action, true);
  // move to redo stack
  redo_actions.emplace_back(move(action));
  last_was_add = false;
}
void ActionStack::redo() {
  assert(canRedo());
  if (!canRedo()) return;
  unique_ptr<Action> action = move(redo_actions.back());
  redo_actions.pop_back();
  action->perform(false);
  tellListeners(*action, false);
  // move to undo stack
  undo_actions.emplace_back(move(action));
  last_was_add = false;
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
      || (undo_actions.back().get() == save_point);
}
void ActionStack::setSavePoint() {
  if (undo_actions.empty()) {
    save_point = nullptr;
  } else {
    save_point = undo_actions.back().get();
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
