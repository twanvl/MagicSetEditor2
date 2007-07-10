//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_SYMBOL
#define HEADER_DATA_SYMBOL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/action_stack.hpp>
#include <util/vector2d.hpp>

DECLARE_POINTER_TYPE(ControlPoint);
DECLARE_POINTER_TYPE(SymbolPart);
DECLARE_POINTER_TYPE(SymbolShape);
DECLARE_POINTER_TYPE(SymbolSymmetry);
DECLARE_POINTER_TYPE(SymbolGroup);
DECLARE_POINTER_TYPE(Symbol);

// ----------------------------------------------------------------------------- : ControlPoint

/// Mode of locking for control points in a bezier curve
/** Specificly: the relation between delta_before and delta_after
 */
enum LockMode
{	LOCK_FREE		///< no locking
,	LOCK_DIR		///< delta_before = x * delta_after
,	LOCK_SIZE		///< delta_before = -delta_after
};

/// Is the segment between two ControlPoints a line or a curve?
enum SegmentMode
{	SEGMENT_LINE
,	SEGMENT_CURVE
};

/// To refer to a specific handle of a control point
enum WhichHandle
{	HANDLE_NONE = 0	///< point is not selected
,	HANDLE_MAIN
,	HANDLE_BEFORE
,	HANDLE_AFTER
};

/// A control point (corner) of a SymbolShape (polygon/bezier-gon)
class ControlPoint : public IntrusivePtrBase<ControlPoint> {
  public:
	Vector2D pos;			///< position of the control point itself
	Vector2D delta_before;	///< delta to bezier control point, for curve before point
	Vector2D delta_after;	///< delta to bezier control point, for curve after point
	SegmentMode segment_before, segment_after;
	LockMode lock;
	
	/// Default constructor
	ControlPoint();
	/// Constructor for straight lines, takes only the position
	ControlPoint(double x, double y);
	/// Constructor for curves lines, takes postions, delta_before, delta_after and lock mode
	ControlPoint(double x, double y, double xb, double yb, double xa, double ya, LockMode lock = LOCK_FREE);
	
	/// Must be called after delta_before/delta_after has changed, enforces lock constraints
	void onUpdateHandle(WhichHandle wh);
	/// Must be called after lock has changed, enforces lock constraints
	void onUpdateLock();

	/// Get a handle of this control point
	Vector2D& getHandle(WhichHandle wh);
	/// Get a handle of this control point that is oposite wh
	Vector2D& getOther(WhichHandle wh);
	
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : Selected handles

/// A specific handle of a ControlPoint
class SelectedHandle {
  public:
	ControlPointP point;  ///< the selected point
	WhichHandle   handle; ///< the selected handle of the point
	
	// SelectedHandle
	SelectedHandle()                                                      : handle(HANDLE_NONE) {}
	SelectedHandle(const WhichHandle&   handle)                           : handle(handle) { assert (handle == HANDLE_NONE); }
	SelectedHandle(const ControlPointP& point, const WhichHandle& handle) : point(point), handle(handle) {}
	
	inline Vector2D& getHandle() const { return  point->getHandle(handle); }
	inline Vector2D& getOther()  const { return  point->getOther (handle); }
	inline void onUpdateHandle() const { return  point->onUpdateHandle(handle); }
	
	/*
	bool operator == (const ControlPointP pnt) const;
	
	bool SelectedHandle::operator == (const ControlPointP pnt) const { return  point == pnt; }
	bool operator == (const WhichHandle& wh) const;
	bool SelectedHandle::operator == (const WhichHandle& wh) const          { return  handle == wh; }
	bool operator ! () const;
	bool SelectedHandle::operator ! () const                         { return  handle == handleNone; }
	*/	
};


// ----------------------------------------------------------------------------- : SymbolPart

/// A part of a symbol, not necesserly a shape
class SymbolPart : public IntrusivePtrVirtualBase {
  public:
	/// Name/label for this part
	String name;
	/// Position and size of the part.
	/** this is the smallest axis aligned bounding box that fits around the part */
	Vector2D min_pos, max_pos;
	
	/// Type of this part
	virtual String typeName() const = 0;
	/// Create a clone of this symbol part
	virtual SymbolPartP clone() const = 0;
	/// Icon for this part
	virtual int icon() const = 0;
	
	/// Convert to SymbolShape?
	virtual       SymbolShape*    isSymbolShape()          { return nullptr; }
	virtual const SymbolShape*    isSymbolShape()    const { return nullptr; }
	/// Convert to SymbolSymmetry?
	virtual       SymbolSymmetry* isSymbolSymmetry()       { return nullptr; }
	virtual const SymbolSymmetry* isSymbolSymmetry() const { return nullptr; }
	/// Convert to SymbolGroup?
	virtual       SymbolGroup*    isSymbolGroup()          { return nullptr; }
	virtual const SymbolGroup*    isSymbolGroup()    const { return nullptr; }
	
	/// Does this part contain another?
	/** also true if this==that*/
	virtual bool isAncestor(const SymbolPart& that) const { return this == &that; }
	
	/// Calculate the position and size of the part (min_pos and max_pos)
	virtual void calculateBounds();
	
	DECLARE_REFLECTION_VIRTUAL();
};

template <> SymbolPartP read_new<SymbolPart>(Reader& reader);

// ----------------------------------------------------------------------------- : SymbolShape

/// How are symbol parts combined with parts below it?
enum SymbolShapeCombine
{	SYMBOL_COMBINE_MERGE
,	SYMBOL_COMBINE_SUBTRACT
,	SYMBOL_COMBINE_INTERSECTION
,	SYMBOL_COMBINE_DIFFERENCE
,	SYMBOL_COMBINE_OVERLAP
,	SYMBOL_COMBINE_BORDER
};

/// A sane mod function, always returns a result in the range [0..size)
inline size_t mod(int a, size_t size) {
	int m = a % (int)size;
	return m >= 0 ? m : m + size;
}

/// A single shape (polygon/bezier-gon) in a Symbol
class SymbolShape : public SymbolPart {
  public:
	/// The points of this polygon
	vector<ControlPointP> points;
	/// How is this part combined with parts below it?
	SymbolShapeCombine combine;
	// Center of rotation, relative to the part, when the part is scaled to [0..1]
	Vector2D rotation_center;
	
	SymbolShape();
	
	virtual String typeName() const;
	virtual SymbolPartP clone() const;
	virtual int icon() const { return combine; }
	virtual       SymbolShape* isSymbolShape()       { return this; }
	virtual const SymbolShape* isSymbolShape() const { return this; }
	
	/// Get a control point, wraps around
	inline ControlPointP getPoint(int id) const {
		return points[mod(id, points.size())];
	}
	
	/// Enforce lock constraints
	void enforceConstraints();
	
	/// Calculate the position and size of the part
	virtual void calculateBounds();
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : SymbolGroup

/// A group of symbol parts
class SymbolGroup : public SymbolPart {
  public:
	vector<SymbolPartP> parts;	///< The parts in this group, first item is on top
	
	SymbolGroup();
	
	virtual String typeName() const;
	virtual SymbolPartP clone() const;
	virtual int icon() const { return SYMBOL_COMBINE_BORDER + 3; }
	virtual       SymbolGroup* isSymbolGroup()       { return this; }
	virtual const SymbolGroup* isSymbolGroup() const { return this; }
	
	virtual bool isAncestor(const SymbolPart& that) const;
	
	virtual void calculateBounds();
	/// re-calculate the bounds, but not of the contained parts
	void calculateBoundsNonRec();
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : SymbolSymmetry

enum SymbolSymmetryType
{	SYMMETRY_ROTATION
,	SYMMETRY_REFLECTION
};

/// A mirror, reflecting the part of the symbol in the group
/** Can handle rotation symmetry with any number of reflections */
class SymbolSymmetry : public SymbolGroup {
  public:
	SymbolSymmetryType kind;	///< What kind of symmetry
	int                copies;	///< How many times is the orignal reflected (including the original itself)
	bool               clip;	///< Clip the orignal so it doesn't intersect the mirror(s)
	Vector2D           center;	///< Center point of the mirror
	Vector2D           handle;	///< A handle pointing in the direction of the original, relative to the center
	
	SymbolSymmetry();
	
	virtual String typeName() const;
	virtual SymbolPartP clone() const;
	virtual int icon() const { return kind + SYMBOL_COMBINE_BORDER + 1; }
	virtual       SymbolSymmetry* isSymbolSymmetry()       { return this; }
	virtual const SymbolSymmetry* isSymbolSymmetry() const { return this; }
	
	String expectedName() const;
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Symbol

/// An editable symbol, consists of any number of SymbolParts
class Symbol : public SymbolGroup {
  public:
	/// Actions performed on this symbol and the parts in it
	ActionStack actions;
	
	DECLARE_REFLECTION();
};

/// A default symbol: a square
SymbolP default_symbol();

// ----------------------------------------------------------------------------- : SymbolView

/// A 'view' of a symbol, is notified when the symbol is updated
/** To listen to events, derived classes should override onAction(const Action&, bool undone)
 */
class SymbolView : public ActionListener {
  public:
	SymbolView();
	~SymbolView();
  
	/// Get the symbol that is currently being viewed
	inline SymbolP getSymbol() { return symbol; }
	/// Change the symbol that is being viewed
	void   setSymbol(const SymbolP& symbol);
	
  protected:
	/// The symbol that is currently being viewed, should not be modified directly!
	SymbolP symbol;
	
	/// Called when another symbol is being viewn (using setSymbol)
	virtual void onChangeSymbol() {}
};


// ----------------------------------------------------------------------------- : EOF
#endif
