//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_SYMBOL
#define HEADER_DATA_ACTION_SYMBOL

/** @file data/action/symbol.hpp
 *
 *  Actions operating on Symbols or whole SymbolParts.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <data/symbol.hpp>

// ----------------------------------------------------------------------------- : Transform symbol part

/// Anything that changes a part
class SymbolPartAction : public Action {};

/// Anything that changes a part as displayed in the part list
class SymbolPartListAction : public SymbolPartAction {};

// ----------------------------------------------------------------------------- : Moving symbol parts

/// Move some symbol parts
class SymbolPartMoveAction : public SymbolPartAction {
  public:
	SymbolPartMoveAction(const set<SymbolPartP>& parts);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  private:
	set<SymbolPartP> parts;		///< Parts to move
	Vector2D delta;				///< How much to move
	Vector2D moved;				///< How much has been moved
	Vector2D min_pos, max_pos;	///< Bounding box of the thing we are moving
  public:
	bool constrain;				///< Constrain movement?
	int snap;					///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Rotating symbol parts

/// Transforming symbol parts using a matrix
class SymbolPartMatrixAction : public SymbolPartAction {
  public:
	SymbolPartMatrixAction(const set<SymbolPartP>& parts, const Vector2D& center);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  protected:
	/// Perform the transformation using the given matrix
	void transform(const Vector2D& mx, const Vector2D& my);
	
	set<SymbolPartP> parts;	///< Parts to transform
	Vector2D center;			///< Center to transform around
};

/// Rotate some symbol parts
class SymbolPartRotateAction : public SymbolPartMatrixAction {
  public:
	SymbolPartRotateAction(const set<SymbolPartP>& parts, const Vector2D& center);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Update this action to rotate to a different angle
	void rotateTo(double newAngle);
	
	/// Update this action to rotate by a deltaAngle
	void rotateBy(double deltaAngle);
	
  private:
	double angle;				///< How much to rotate?
  public:
	bool constrain;				///< Constrain movement?
};


// ----------------------------------------------------------------------------- : Shearing symbol parts

/// Shear some symbol parts
class SymbolPartShearAction : public SymbolPartMatrixAction {
  public:
	SymbolPartShearAction(const set<SymbolPartP>& parts, const Vector2D& center);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Change shear by a given amount
	void move(const Vector2D& deltaShear);
	
  private:
	Vector2D shear;				///< Shearing, shear.x == 0 || shear.y == 0
	Vector2D moved;
	void shearBy(const Vector2D& shear);
  public:
//	bool constrain;				///< Constrain movement?
	int snap;					///< Snap to grid?
};


// ----------------------------------------------------------------------------- : Scaling symbol parts

/// Scale some symbol parts
class SymbolPartScaleAction : public SymbolPartAction {
  public:
	SymbolPartScaleAction(const set<SymbolPartP>& parts, int scaleX, int scaleY);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Change min and max coordinates
	void move(const Vector2D& delta_min, const Vector2D& delta_max);
	/// Update the action's effect
	void update();
	
  private:
	set<SymbolPartP> parts;					///< Parts to scale
	Vector2D old_min,      old_size;		///< the original pos/size
	Vector2D new_real_min, new_real_size;	///< the target pos/sizevoid shearBy(const Vector2D& shear)
	Vector2D new_min,      new_size;		///< the target pos/size after applying constrains
	int scaleX, scaleY;						///< to what corner are we attached?
	/// Transform everything in the parts
	void transformAll();
	/// Transform a single vector
	inline Vector2D transform(const Vector2D& v);
  public:
	bool constrain;				///< Constrain movement?
	int snap;					///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Change combine mode

/// Change the name of a symbol part
class CombiningModeAction : public SymbolPartListAction {
  public:
	// All parts must be SymbolParts
	CombiningModeAction(const set<SymbolPartP>& parts, SymbolShapeCombine mode);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	vector<pair<SymbolShapeP,SymbolShapeCombine> > parts;	///< Affected parts with new combining modes
};

// ----------------------------------------------------------------------------- : Change name

/// Change the name of a symbol part
class SymbolPartNameAction : public SymbolPartListAction {
  public:
	SymbolPartNameAction(const SymbolPartP& part, const String& name);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	SymbolPartP part;	///< Affected part
	String part_name;	///< New name
};

// ----------------------------------------------------------------------------- : Add symbol part

/// Adding a part to a symbol, added at the front of the list (drawn on top)
class AddSymbolPartAction : public SymbolPartListAction {
  public:
	AddSymbolPartAction(Symbol& symbol, const SymbolPartP& part);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	Symbol& symbol;		///< Symbol to add the part to
	SymbolPartP part;	///< Part to add
};

// ----------------------------------------------------------------------------- : Remove symbol part

/// Removing parts from a symbol
class RemoveSymbolPartsAction : public SymbolPartListAction {
  public:
	RemoveSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	Symbol& symbol;
	/// Removed parts and their positions, sorted by ascending pos
	vector<pair<SymbolPartP, size_t> > removals;
};

// ----------------------------------------------------------------------------- : Duplicate symbol parts

/// Duplicating parts in a symbol
class DuplicateSymbolPartsAction : public SymbolPartListAction {
  public:
	DuplicateSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Fill a set with all the new parts
	void getParts(set<SymbolPartP>& parts);
	
  private:
	Symbol& symbol;
	/// Duplicates of parts and their positions, sorted by ascending pos
	vector<pair<SymbolPartP, size_t> > duplications;
};


// ----------------------------------------------------------------------------- : Reorder symbol parts

/// Change the position of a part in a symbol, by swapping two parts.
class ReorderSymbolPartsAction : public SymbolPartListAction {
  public:
	ReorderSymbolPartsAction(Symbol& symbol, size_t part_id1, size_t part_id2);
  
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	Symbol& symbol;				///< Symbol to swap the parts in
  public:
	size_t part_id1, part_id2;	///< Indeces of parts to swap
};


// ----------------------------------------------------------------------------- : EOF
#endif
