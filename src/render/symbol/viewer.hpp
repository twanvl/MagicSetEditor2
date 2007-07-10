//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_VIEWER
#define HEADER_GUI_SYMBOL_VIEWER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <data/symbol.hpp>
#include <gfx/bezier.hpp>

// ----------------------------------------------------------------------------- : Simple rendering

/// Render a Symbol to an Image
Image render_symbol(const SymbolP& symbol, double border_radius = 0.05, int size = 100, bool editing_hints = false);

// ----------------------------------------------------------------------------- : Symbol Viewer

enum HighlightStyle
{	HIGHLIGHT_BORDER     = 0x01
,	HIGHLIGHT_INTERIOR   = 0x02
,	HIGHLIGHT_LESS       = 0x10
,	HIGHLIGHT_BORDER_DOT = HIGHLIGHT_BORDER | HIGHLIGHT_LESS
};

/// Class that knows how to draw a symbol
class SymbolViewer : public SymbolView {
  public:
	// --------------------------------------------------- : Data
	SymbolViewer(const SymbolP& symbol, bool editing_hints, double size = 500, double border_radius = 0.05);
	
	// drawing
	double border_radius;
	bool editing_hints;
	
	// --------------------------------------------------- : Point translation
	
	void setZoom(double zoom);
	
	Rotation rotation; ///< Object that handles rotation, scaling and translation
	Matrix2D multiply; ///< Scaling/rotation of actual parts
	Vector2D origin;   ///< Origin of parts
	
	// --------------------------------------------------- : Drawing
	
	/// Draw the symbol to a dc
	void draw(DC& dc);
	
	void highlightPart(DC& dc, const SymbolPart& part,    HighlightStyle style);
	void highlightPart(DC& dc, const SymbolShape& shape,  HighlightStyle style);
	void highlightPart(DC& dc, const SymbolSymmetry& sym, HighlightStyle style);
	void highlightPart(DC& dc, const SymbolGroup& group,  HighlightStyle style);
	
	void drawEditingHints(DC& dc);
	
	void onAction(const Action&, bool) {}
	
	
  private:
	typedef shared_ptr<wxMemoryDC> MemoryDCP;
	/// Inside a reflection?
	int in_symmetry;
	
	/// Combine a symbol part with the dc
	void SymbolViewer::combineSymbolPart(DC& dc, const SymbolPart& part, bool& paintedSomething, bool& buffersFilled, bool allow_overlap, MemoryDCP& borderDC, MemoryDCP& interiorDC);
	
	/// Combines a symbol part with what is currently drawn, the border and interior are drawn separatly
	/** directB/directI are true if the border/interior is the screen dc, false if it
	 *  is a temporary 1 bit one
	 */
	void combineSymbolShape(const SymbolShape& part, DC& border, DC& interior, bool directB, bool directI);
	
	/// Draw a symbol part, draws the border and the interior to separate DCs
	/** The DCs may be null. directB should be true when drawing the border directly to the screen.
	 *  The **Col parameters give the color to use for the (interior of) the border and the interior
	 *  default should be white (255) border and black (0) interior.
	 */
	void drawSymbolShape(const SymbolShape& shape, DC* border, DC* interior, unsigned char borderCol, unsigned char interiorCol, bool directB, bool oppB);
/*	
	// ------------------- Bezier curve calculation
	
	// Calculate the points on a bezier curve between p0 and p1
	// Stores the Points in p_out, at most count points are stored
	// after this call p_out points to just beyond the last point
	void calcBezierPoint(const ControlPointP& p0, const ControlPointP& p1, wxPoint*& p_out, UInt count);
	
	// Subdivide a bezier curve by adding at most count points
	//   p0 = c(t0), p1 = c(p1)
	// subdivides linearly between t0 and t1, and only when necessary
	// adds points to p_out and increments the pointer when a point is added
	void calcBezierOpt(const BezierCurve& c, const Vector2D& p0, const Vector2D& p1, double t0, double t1, wxPoint*& p_out, UInt count);
*/};

// ----------------------------------------------------------------------------- : EOF
#endif
