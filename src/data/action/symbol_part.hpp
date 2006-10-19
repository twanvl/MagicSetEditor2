//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_SYMBOL_PART
#define HEADER_DATA_ACTION_SYMBOL_PART

/** @file data/action/symbol_part.hpp
 *
 *  Actions operating on the insides of SymbolParts (ControlPoints and the like).
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <data/symbol.hpp>

// ----------------------------------------------------------------------------- : Utility

/// Constrain a vector to be horizontal, vertical or diagonal
/// If constraint==false does nothing
Vector2D constrainVector(const Vector2D& v, bool constrain = true, bool onlyDiagonal = false);

// ----------------------------------------------------------------------------- : Move control point

/// Moving a control point in a symbol
class ControlPointMoveAction : public Action {
  public:
	ControlPointMoveAction(const set<ControlPointP>& points);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  private:
	set<ControlPointP> points;  ///< Points to move
	vector<Vector2D> oldValues; ///< Their old positions
	Vector2D delta;				///< Amount we moved
  public:
	bool constrain;				///< Constrain movement?
};

// ----------------------------------------------------------------------------- : Move handle

/// Moving a handle(before/after) of a control point in a symbol
class HandleMoveAction : public Action {
  public:
	HandleMoveAction(const SelectedHandle& handle);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  private:
	SelectedHandle handle;		///< The handle to move
	Vector2D old_handle;		///< Old value of this handle
	Vector2D old_other;			///< Old value of other handle, needed for contraints
	Vector2D delta;				///< Amount we moved
  public:
	bool constrain;				///< Constrain movement?
};

// ----------------------------------------------------------------------------- : Segment mode

/// Utility class to update a control point
class ControlPointUpdate {
  public:
	ControlPointUpdate(const ControlPointP& pnt);
	
	/// Perform or undo an update on this control point
	void perform();
	
	/// Other value that is swapped with the current one.
	/// Should be changed to make perform have an effect
	ControlPoint other;
	/// The point that is to be changed, should not be updated before perform()
	ControlPointP point;
};


/// Changing a line to a curve and vice versa
class SegmentModeAction : public Action {
  public:
	SegmentModeAction(const ControlPointP& p1, const ControlPointP& p2, SegmentMode mode);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  protected:
	ControlPointUpdate point1, point2;
};

// ----------------------------------------------------------------------------- : Locking mode

/// Locking a control point
class LockModeAction : public Action {
  public:
	LockModeAction(const ControlPointP& p, LockMode mode);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	ControlPointUpdate point;	///< The affected point
};

// ----------------------------------------------------------------------------- : Move curve

/// Dragging a curve; also coverts lines to curves
/** Inherits from SegmentModeAction because it also has that effect
 */
class CurveDragAction : public SegmentModeAction {
  public:
	CurveDragAction(const ControlPointP& point1, const ControlPointP& point2);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
		
	// Move the curve by this much, it is grabbed at time t
	void move(const Vector2D& delta, double t);
};

// ----------------------------------------------------------------------------- : Add control point

/// Insert a new point in a symbol part
class ControlPointAddAction : public Action {
  public:
	/// Insert a new point in part, after position insertAfter_, at the time t on the segment
	ControlPointAddAction(const SymbolPartP& part, UInt insert_after, double t);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	inline ControlPointP getNewPoint() const { return new_point; }
	
  private:
	SymbolPartP   part;					///< SymbolPart we are in
	ControlPointP new_point;				///< The point to insert
	UInt          insert_after;			///< Insert after index .. in the array
	ControlPointUpdate point1, point2;	///< Update the points around the new point
};

// ----------------------------------------------------------------------------- : Remove control point

/// Action that removes any number of points from a symbol part
/// TODO: If less then 3 points are left removes the entire part!
Action* controlPointRemoveAction(const SymbolPartP& part, const set<ControlPointP>& toDelete);


// ----------------------------------------------------------------------------- : EOF
#endif
