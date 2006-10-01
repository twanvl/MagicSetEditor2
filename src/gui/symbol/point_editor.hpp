//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_POINT_EDITOR
#define HEADER_GUI_SYMBOL_POINT_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>

class HandleMoveAction;
class ControlPointMoveAction;
class CurveDragAction;

// ----------------------------------------------------------------------------- : SymbolPointEditor


// Symbol editor for editing control points and handles
class SymbolPointEditor : public SymbolEditorBase {
  public:
	SymbolPointEditor(SymbolControl* control, const SymbolPartP& part);
	
	// --------------------------------------------------- : Drawing
	
	virtual void draw(DC& dc);
	
  private:
	/// Draws a gradient on the selected line to indicate curve dragging
	void drawHoveredLine(DC& dc);
	/// Draw all handles belonging to selected points
	void drawHandles(DC& dc);
	/// Draws the point to be inserted
	void drawNewPoint(DC& dc);
	/// Draw a single control point
	void drawControlPoint(DC& dc, const ControlPoint& p, bool drawHandleBefore, bool drawHandleAfter);
	/// Draws a handle as a box
	void drawHandleBox(DC& dc, UInt px, UInt py, bool active);
	/// Draws a handle as a circle
	void drawHandleCircle(DC& dc, UInt px, UInt py);
	
	enum WhichPen {
		PEN_NORMAL,		//^ Pen for normal handles
		PEN_HOVER,		//^ Pen for hovered handles
		PEN_LINE,		//^ Pen for the line to handles
		PEN_MAIN,		//^ Pen for the main handle
		PEN_NEW_POINT	//^ Pen for the new point
	};
	/// Retrieve a pen for the drawing of parts of handles
	wxPen handlePen(WhichPen p, LockMode lock);
	/// Retrieve a pen for the drawing of other things
	wxPen otherPen(WhichPen p);
	
  public:
	// --------------------------------------------------- : UI
	
	virtual void initUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent& e);
	virtual void onCommand(int id);
	virtual int modeToolId();
		
	// --------------------------------------------------- : Mouse events
	
	virtual void onLeftDown  (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onLeftUp    (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onLeftDClick(const Vector2D& pos, wxMouseEvent& ev);
	virtual void onMouseMove(const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	virtual void onMouseDrag(const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	
	// --------------------------------------------------- : Other events
	
	virtual void onKeyChange(wxKeyEvent& ev);
	virtual void onChar(wxKeyEvent& ev);
	virtual bool isEditing();
	
  private:
	// --------------------------------------------------- : Data
	
	// The symbol part we are editing
	SymbolPartP part;
	
	// Actions in progress
	// All are owned by the action stack, or they are 0
	HandleMoveAction*       handleMoveAction;
	ControlPointMoveAction* controlPointMoveAction;
	CurveDragAction*        curveDragAction;
	
	// Selection
	enum Selection {
		SELECTED_NONE,		//^ no selection
		SELECTED_POINTS,	//^ some points are selected
		SELECTED_HANDLE,	//^ a handle is selected
		SELECTED_LINE,		//^ a line is selected
		SELECTED_NEW_POINT	//^ a new point on a line (used for hovering)
	};
	Selection          selection;
	//   points
	set<ControlPointP> selectedPoints;
	//   handle
	SelectedHandle     selectedHandle;
	//   line
	ControlPointP      selectedLine1, selectedLine2; // selected the line between these points
	double             selectedLineT;                // time on the line of the selection
	
	// Mouse feedback
	Selection      hovering;
	//   handle
	SelectedHandle hoverHandle; // the handle currently under the cursor
	//   new point
	Vector2D       newPoint;
	//   line
	ControlPointP  hoverLine1, hoverLine2; // hovering on the line between these points
	double         hoverLineT;
	int            hoverLine1Idx; // index of hoverLine1 in the list of points
	
	// Gui stock
	wxBitmap background;
	wxCursor pointSelect, pointAdd, pointCurve, pointMove;
		
	// --------------------------------------------------- : Selection
	
	/// Clears the selection
	void selectNothing();
	/// Select a point, if toggle then toggles the selection of the point
	void selectPoint(const ControlPointP& point, bool toggle);
	void selectHandle(const SelectedHandle& h, const wxMouseEvent& keystate);
	void selectLine(const wxMouseEvent& keystate);
	
	/// Is a point selected?
	bool pointSelected(const ControlPointP& pnt);
	bool pointSelected(const ControlPoint&  pnt);
	/// Is the mouse pointer above a point?
	bool pointHovered(const ControlPointP& pnt);
	bool pointHovered(const ControlPoint&  pnt);
	/// Is the mouse pointer above a handle of a point?
	bool handleHovered(const ControlPointP& pnt, WhichHandle wh);
	bool handleHovered(const ControlPoint&  pnt, WhichHandle wh);
	
	// --------------------------------------------------- : Actions
	
	// Finalize actions; new events start new actions
	void resetActions();
	void deleteSelection();
	void onChangeSegment(SegmentMode mode);
	void onChangeLock   (LockMode mode);
	
	// --------------------------------------------------- : Finding items
	
	/// Finds the item that is currently being hovered, stores the results in hover*
	void findHoveredItem(const Vector2D& pos, bool altDown);

	/// Is the specified position on a curve?
	/// If so, sets hoverLine*, and set hovering=hoveringLine
	bool checkPosOnCurve(const Vector2D& pos);
	
	/// Finds a handle at or near pos
	SelectedHandle findHandle(const Vector2D& pos);
	
	/// Is the manhatan distance between two points <= range?
	bool inRange(const Vector2D& a, const Vector2D& b, double range);
};


// ----------------------------------------------------------------------------- : EOF
#endif
