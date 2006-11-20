//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_BASIC_SHAPE_EDITOR
#define HEADER_GUI_SYMBOL_BASIC_SHAPE_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>

class wxSpinCtrl;

// ----------------------------------------------------------------------------- : SymbolBasicShapeEditor

/// Editor for drawing basic shapes such as rectangles and polygons
class SymbolBasicShapeEditor : public SymbolEditorBase {
  public:
	SymbolBasicShapeEditor(SymbolControl* control);
	
	// --------------------------------------------------- : Drawing
	
	virtual void draw(DC& dc);
	
	// --------------------------------------------------- : UI
	
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	virtual int modeToolId();
	
	// --------------------------------------------------- : Mouse events
	
	virtual void onLeftDown   (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onLeftUp     (const Vector2D& pos, wxMouseEvent& ev);
	virtual void onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev);
	
	// --------------------------------------------------- : Other events
	
	virtual void onKeyChange(wxKeyEvent& ev);
	
	virtual bool isEditing();
	
	// --------------------------------------------------- : Data
  private:
	int mode;
	SymbolPartP shape;
	Vector2D start;
	Vector2D end;
	bool drawing;
	// controls
	wxSpinCtrl*   sides;
	wxStaticText* sidesL;
	
	/// Cancel the drawing
	void stopActions();
	
	/// Make the shape
	/**  when centered: a = center, b-a = radius
	 *   otherwise:     a = top left, b = bottom right
	 */
	void makeShape(const Vector2D& a, const Vector2D& b, bool constrained, bool centered);
	
	/// Make the shape, centered in c, with radius r
	void makeCenteredShape(const Vector2D& c, Vector2D r, bool constrained);
};

// ----------------------------------------------------------------------------- : EOF
#endif
