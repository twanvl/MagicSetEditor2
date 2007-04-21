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

/// A control point (corner) of a SymbolPart (polygon/bezier-gon)
class ControlPoint {
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

/// How are symbol parts combined with parts below it?
enum SymbolPartCombine
{	PART_MERGE
,	PART_SUBTRACT
,	PART_INTERSECTION
,	PART_DIFFERENCE
,	PART_OVERLAP
,	PART_BORDER
};

/// A sane mod function, always returns a result in the range [0..size)
inline size_t mod(int a, size_t size) {
	int m = a % (int)size;
	return m >= 0 ? m : m + size;
}

/// A single part (polygon/bezier-gon) in a Symbol
class SymbolPart {
  public:
	/// The points of this polygon
	vector<ControlPointP> points;
	/// Name/label for this part
	String name;
	/// How is this part combined with parts below it?
	SymbolPartCombine combine;
	// Center of rotation, relative to the part, when the part is scaled to [0..1]
	Vector2D rotation_center;
	/// Position and size of the part
	/// this is the smallest axis aligned bounding box that fits around the part
	Vector2D min_pos, max_pos;
	
	SymbolPart();
	
	/// Create a clone of this symbol part
	SymbolPartP clone() const;
	
	/// Get a control point, wraps around
	inline ControlPointP getPoint(int id) const {
		return points[mod(id, points.size())];
	}
	
	/// Enforce lock constraints
	void enforceConstraints();
	
	/// Calculate the position and size of the part
	void calculateBounds();
	
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : Symbol

/// An editable symbol, consists of any number of SymbolParts
class Symbol {
  public:
	/// The parts of this symbol
	vector<SymbolPartP> parts;
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
