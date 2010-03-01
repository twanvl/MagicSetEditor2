//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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

/// Anything that changes one or more parts
class SymbolPartAction : public Action {};

/// Anything that changes a set of parts
class SymbolPartsAction : public SymbolPartAction {
  public:
	SymbolPartsAction(const set<SymbolPartP>& parts);
	
	const set<SymbolPartP> parts;	///< Affected parts
};

/// Anything that changes a part as displayed in the part list
class SymbolPartListAction : public SymbolPartAction {};

// ----------------------------------------------------------------------------- : Moving symbol parts

/// Move some symbol parts
class SymbolPartMoveAction : public SymbolPartsAction {
  public:
	SymbolPartMoveAction(const set<SymbolPartP>& parts, const Vector2D& delta = Vector2D());
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  private:
	Vector2D delta;		///< How much to move
	Vector2D moved;		///< How much has been moved
	Bounds   bounds;	///< Bounding box of the thing we are moving
	
	void movePart(SymbolPart& part); ///< Move a single part
  public:
	bool constrain;		///< Constrain movement?
	int snap;			///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Rotating symbol parts

/// Transforming symbol parts using a matrix
class SymbolPartMatrixAction : public SymbolPartsAction {
  public:
	SymbolPartMatrixAction(const set<SymbolPartP>& parts, const Vector2D& center);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  protected:
	/// Perform the transformation using the given matrix
	void transform(const Matrix2D& m);
	void transform(SymbolPart& part, const Matrix2D& m);
	
	Vector2D center; ///< Center to transform around
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
class SymbolPartScaleAction : public SymbolPartsAction {
  public:
	SymbolPartScaleAction(const set<SymbolPartP>& parts, int scaleX, int scaleY);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	/// Change min and max coordinates
	void move(const Vector2D& delta_min, const Vector2D& delta_max);
	/// Update the action's effect
	void update();
	
  private:
	Vector2D old_min,      old_size;		///< the original pos/size
	Vector2D new_real_min, new_real_size;	///< the target pos/sizevoid shearBy(const Vector2D& shear)
	Vector2D new_min,      new_size;		///< the target pos/size after applying constrains
	int scaleX, scaleY;						///< to what corner are we attached?
	/// Transform everything in the parts
	void transformAll();
	void transformPart(SymbolPart&);
	/// Transform a single vector
	inline Vector2D transform(const Vector2D& v);
  public:
	bool constrain;				///< Constrain movement?
	int snap;					///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Change combine mode

/// Change the name of a symbol part
class CombiningModeAction : public SymbolPartsAction {
  public:
	// All parts must be SymbolParts
	CombiningModeAction(const set<SymbolPartP>& parts, SymbolShapeCombine mode);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	void add(const SymbolPartP&, SymbolShapeCombine mode);
	vector<pair<SymbolShapeP,SymbolShapeCombine> > parts;	///< Affected parts with new combining modes
};

// ----------------------------------------------------------------------------- : Change name

/// Change the name of a symbol part
class SymbolPartNameAction : public SymbolPartAction {
  public:
	SymbolPartNameAction(const SymbolPartP& part, const String& name, size_t old_cursor, size_t new_cursor);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	virtual bool merge(const Action& action);
	
  public:
	SymbolPartP part;	///< Affected part
	String part_name;	///< New name
	size_t old_cursor;	///< Cursor position
	size_t new_cursor;	///< Cursor position
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
	/// Check for removals in a group
	void check(SymbolGroup& group, const set<SymbolPartP>& parts);
  public:
	/// A removal step
	struct Removal {
		inline Removal(SymbolGroup& parent, size_t pos, const SymbolPartP& removed)
			: parent(&parent), pos(pos), removed(removed)
		{}
		SymbolGroup* parent;
		size_t       pos;
		SymbolPartP  removed;
	};
  private:
	/// Removed parts, sorted by ascending pos
	vector<Removal> removals;
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

/// Change the position of a part in a symbol, by moving a part.
class ReorderSymbolPartsAction : public SymbolPartListAction {
  public:
	ReorderSymbolPartsAction(SymbolGroup& old_parent, size_t old_position, SymbolGroup& new_parent, size_t new_position);
  
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	SymbolGroup* old_parent, *new_parent;///< Parents to move from and to
  public:
	size_t old_position, new_position;	///< Positions to move from and to
};

/// Break up a single group, and put its contents at a specific position
class UngroupReorderSymbolPartsAction : public SymbolPartListAction {
  public:
	/// Remove all the given groups
	UngroupReorderSymbolPartsAction(SymbolGroup& group_parent, size_t group_pos, SymbolGroup& target_parent, size_t target_pos);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  private:
	SymbolGroup& group_parent;
	size_t       group_pos;
	SymbolGroupP group;      ///< Group to destroy
	SymbolGroup& target_parent;
	size_t       target_pos;
};

// ----------------------------------------------------------------------------- : Group symbol parts

/// Group multiple symbol parts together
class GroupSymbolPartsActionBase : public SymbolPartListAction {
  public:
	GroupSymbolPartsActionBase(SymbolGroup& root);
	
	virtual void perform(bool to_undo);
	
  protected:
	SymbolGroup&        root;			///< Symbol or group to group stuff in
	vector<SymbolPartP> old_part_list;	///< Old part list of the symbol
};

/// Group multiple symbol parts together
class GroupSymbolPartsAction : public GroupSymbolPartsActionBase {
  public:
	GroupSymbolPartsAction(SymbolGroup& root, const set<SymbolPartP>& parts, const SymbolGroupP& group);
	
	virtual String getName(bool to_undo) const;
  private:
	SymbolGroupP group;
};

/// Break up one or more SymbolGroups
class UngroupSymbolPartsAction : public GroupSymbolPartsActionBase {
  public:
	/// Remove all the given groups
	UngroupSymbolPartsAction(SymbolGroup& root, const set<SymbolPartP>& groups);
	
	virtual String getName(bool to_undo) const;
};

// ----------------------------------------------------------------------------- : EOF
#endif
