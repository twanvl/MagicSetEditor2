//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
	/// Update this action to move some more
	void move(const Vector2D& delta);
	
  private:
	set<SymbolPartP> parts;		///< Parts to move
	Vector2D delta;				///< How much to move
	Vector2D moved;				///< How much has been moved
  public:
	bool constrain;				///< Constrain movement?
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
	
	set<SymbolPartP> parts;		///< Parts to transform
	Vector2D center;			///< Center to transform around
};

/// Rotate some symbol parts
class SymbolPartRotateAction : public SymbolPartMatrixAction {
  public:
	SymbolPartRotateAction(const set<SymbolPartP>& parts, const Vector2D& center);
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
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
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
	/// Change shear by a given amount
	void move(const Vector2D& deltaShear);
	
  private:
	Vector2D shear;				///< Shearing, shear.x == 0 || shear.y == 0
	void shearBy(const Vector2D& shear);
  public:
	bool constrain;				///< Constrain movement?
};


// ----------------------------------------------------------------------------- : Scaling symbol parts

/// Scale some symbol parts
class SymbolPartScaleAction : public SymbolPartAction {
  public:
	SymbolPartScaleAction(const set<SymbolPartP>& parts, int scaleX, int scaleY);
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
	/// Change min and max coordinates
	void move(const Vector2D& deltaMin, const Vector2D& deltaMax);
	/// Update the action's effect
	void update();
	
  private:
	set<SymbolPartP> parts;				///< Parts to scale
	Vector2D oldMin,     oldSize;		///< the original pos/size
	Vector2D newRealMin, newRealSize;	///< the target pos/sizevoid shearBy(const Vector2D& shear)
	Vector2D newMin,     newSize;		///< the target pos/size after applying constrains
	int scaleX, scaleY;					///< to what corner are we attached?
	/// Transform everything in the parts
	void transformAll();
	/// Transform a single vector
	inline Vector2D transform(const Vector2D& v) {
		return (v - oldMin).div(oldSize).mul(newSize) + newMin;
	}
  public:
	bool constrain;						///< Constrain movement?
};

// ----------------------------------------------------------------------------- : Change combine mode

/// Change the name of a symbol part
class CombiningModeAction : public SymbolPartListAction {
  public:
	CombiningModeAction(const set<SymbolPartP>& parts, SymbolPartCombine mode);
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
  private:
	vector<pair<SymbolPartP,SymbolPartCombine> > parts;	///< Affected parts with new combining modes
};

// ----------------------------------------------------------------------------- : Change name

/// Change the name of a symbol part
class SymbolPartNameAction : public SymbolPartListAction {
  public:
	SymbolPartNameAction(const SymbolPartP& part, const String& name);
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
  private:
	SymbolPartP part;		///< Affected part
	String partName;		///< New name
};

// ----------------------------------------------------------------------------- : Add symbol part

/// Adding a part to a symbol, added at the front of the list (drawn on top)
class AddSymbolPartAction : public SymbolPartListAction {
  public:
	AddSymbolPartAction(Symbol& symbol, const SymbolPartP& part);
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
  private:
	Symbol& symbol;			///< Symbol to add the part to
	SymbolPartP part;		///< Part to add
};

// ----------------------------------------------------------------------------- : Remove symbol part

/// Removing parts from a symbol
class RemoveSymbolPartsAction : public SymbolPartListAction {
  public:
	RemoveSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts);
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
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
	
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
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
	ReorderSymbolPartsAction(Symbol& symbol, size_t partId1, size_t partId2);
  
	virtual String getName(bool toUndo) const;
	virtual void perform(bool toUndo);
	
  private:
	Symbol& symbol;				///< Symbol to swap the parts in
  public:
	size_t partId1, partId2;	///< Indeces of parts to swap
};


// ----------------------------------------------------------------------------- : EOF
#endif
