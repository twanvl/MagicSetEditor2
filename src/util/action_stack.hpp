//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ACTION_STACK
#define HEADER_UTIL_ACTION_STACK

// ----------------------------------------------------------------------------- : Includes

#include <vector>
#include "string.hpp"

// ----------------------------------------------------------------------------- : Action

/// Base class for actions that can be stored in an ActionStack.
/** An action is something that can be done to modify an object.
 *  It must store the necessary information to also undo the action.
 */
class Action {
  public:
	virtual ~Action() {};
	
	/// Name of the action, for use in strings like "Undo <name>"
	virtual String getName(bool toUndo) const = 0;
	
	/// Perform the action
	/** @param toUndo if true, undo the action instead of doing it
	 *
	 *  Must be implemented in derived class.
	 *
	 *  Perform will only ever be called alternatingly with toUndo = true/false,
	 *  the first time with toUndo = false
	 */
	virtual void perform(bool toUndo) = 0;
	
	/// Try to merge another action to the end of this action.
	/** Either: return false and do nothing
	 *  Or: return true and change this action to incorporate both actions
	 */
	virtual bool merge(const Action* action) { return false; }
};

// ----------------------------------------------------------------------------- : Action listeners

/// Base class/interface for objects that listen to actions
class ActionListener {
  public:
	virtual void onAction(const Action& a) = 0;
};

// ----------------------------------------------------------------------------- : Action stack

/// A stack of actions that can be done and undone.
/** This class handles the undo and redo functionality of a particular object.
 * 
 *  This class also takes on the role of Observable, ActionListeners can register themselfs.
 *  They will be notified when an action is added.
 */
class ActionStack {
  public:
	ActionStack();
	~ActionStack();
	
	/// Add an action to the stack, and perform that action.
	/// Tells all listeners about the action.
	/// The ActionStack takes ownership of the action
	void add(Action* action, bool allowMerge = true);
	
	/// Undoes the last action that was (re)done
	/// @pre canUndo()
	void undo();
	/// Redoes the last action that was undone
	/// @pre canRedo()
	void redo();
	
	/// Is undoing possible?
	bool canUndo() const;
	/// Is redoing possible?
	bool canRedo() const;
	
	/// Name of the action that will be undone next, in the form "Undo <Action>"
	/// If there is no action to undo returns "Undo"
	String undoName() const;
	/// Name of the action that will be redone next "Redo <Action>"
	/// If there is no action to undo returns "Redo"
	String redoName() const;
	
	/// Is the file currently at a 'savepoint'?
	/// This is the last point at which the file was saved
	bool atSavePoint() const;
	/// Indicate that the file is at a savepoint.
	void setSavePoint();
	
	/// Add an action listener
	void addListener(ActionListener* listener);
	/// Remove an action listener
	void removeListener(ActionListener* listener);
	/// Tell all listeners about an action
	void tellListeners(const Action&);
	
  private:
	/// Actions to be undone
	/// Owns the action objects!
	vector<Action*> undoActions;
	/// Actions to be redone
	/// Owns the action objects!
	vector<Action*> redoActions;
	/// Point at which the file was saved, corresponds to the top of the undo stack at that point
	Action* savePoint;
	/// Objects that are listening to actions
	vector<ActionListener*> listeners;
};


// ----------------------------------------------------------------------------- : Utilities

/// Tests if variable has the type Type
/** Uses dynamic cast, so Type must have a virtual function.
 */
#define TYPE_CASE_(variable, Type)									\
		if (dynamic_cast<const Type*>(&variable))

/// Tests if variable has the type Type. If this is the case, makes
/// variable have type Type inside the statement
/** Uses dynamic cast, so Type must have a virtual function.
 */
#define TYPE_CASE(variable, Type)									\
		pair<const Type*,bool> Type##variable						\
			(dynamic_cast<const Type*>(&variable), true);			\
		if (Type##variable.first)									\
			for (const Type& variable = *Type##variable.first ;		\
				 Type##variable.second ;							\
				 Type##variable.second = false)


// ----------------------------------------------------------------------------- : EOF
#endif
