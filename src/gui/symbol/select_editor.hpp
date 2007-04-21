//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_SELECT_EDITOR
#define HEADER_GUI_SYMBOL_SELECT_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>

DECLARE_POINTER_TYPE(SymbolPartMoveAction);
DECLARE_POINTER_TYPE(SymbolPartScaleAction);
DECLARE_POINTER_TYPE(SymbolPartRotateAction);
DECLARE_POINTER_TYPE(SymbolPartShearAction);

// ----------------------------------------------------------------------------- : SymbolSelectEditor

/// Editor that allows the user to select symbol parts
class SymbolSelectEditor : public SymbolEditorBase {
  public:
	SymbolSelectEditor(SymbolControl* control, bool rotate);
	
	// --------------------------------------------------- : Drawing
	
	virtual void draw(DC& dc);
	
  private:
	/// Draw handles on all sides
	void drawHandles(DC& dc);
	/// Draw a handle, dx and dy indicate the side, can be {-1,0,1}
	void drawHandle(DC& dc, int dx, int dy);
	
	/// Draw the rotation center
	void drawRotationCenter(DC& dc, const Vector2D& pos);
	
  public:
	// --------------------------------------------------- : UI
	
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	virtual int modeToolId();
	
	// --------------------------------------------------- : Mouse events
	
	virtual void onLeftDown   (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onLeftDClick (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onLeftUp     (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	virtual void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	
	// --------------------------------------------------- : Other events
	
	virtual void onKeyChange  (wxKeyEvent& ev);
	virtual void onChar       (wxKeyEvent& ev);
	
	virtual bool isEditing();
	
  private:
	// The part under the mouse cursor
	SymbolPartP highlightPart;
	// Actions
	// All are either owned by the symbol's action stack or equal 0
	SymbolPartMoveAction*   moveAction;
	SymbolPartScaleAction*  scaleAction;
	SymbolPartRotateAction* rotateAction;
	SymbolPartShearAction*  shearAction;
	// Bounding box of selection
	Vector2D minV, maxV;
	// Where is the rotation center?
	Vector2D center;
	// At what angle is the handle we started draging for rotation
	double startAngle;
	// what side are we dragging/rotating on?
	int scaleX, scaleY;
	// have we dragged?
	bool have_dragged;
	// Do we want to rotate?
	bool rotate;
	// Graphics assets
	wxCursor cursorRotate;
	wxCursor cursorShearX;
	wxCursor cursorShearY;
	Bitmap handleRotateTL, handleRotateTR, handleRotateBL, handleRotateBR;
	Bitmap handleShearX, handleShearY;
	Bitmap handleCenter;

	/// Is the mouse on a scale/rotate handle?
	bool onHandle(const Vector2D& mpos, int dx, int dy);
	
	/// Is the mouse on any handle?
	/** Returns the handle coordinates [-1..1] in d?Out
	 */
	bool onAnyHandle(const Vector2D& mpos, int* dxOut, int* dyOut);
	
	/// Angle between center and pos
	double angleTo(const Vector2D& pos);
	
	/// Return the position of a handle, dx,dy in <-1, 0, 1>
	Vector2D handlePos(int dx, int dy);
	
	/// Find the first part at the given position
	SymbolPartP findPart(const Vector2D& pos);
	
	/// Update minV and maxV to be the bounding box of the selected_parts
	/// Updates center to be the rotation center of the parts
	void updateBoundingBox();
	
	/// Reset all the actions to 0
	void resetActions();
};

// ----------------------------------------------------------------------------- : EOF
#endif
