//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/basic_shape_editor.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <data/settings.hpp>
#include <data/action/symbol.hpp>
#include <data/action/symbol_part.hpp>
#include <wx/spinctrl.h>

// ----------------------------------------------------------------------------- : SymbolBasicShapeEditor

SymbolBasicShapeEditor::SymbolBasicShapeEditor(SymbolControl* control)
	: SymbolEditorBase(control)
	, mode(ID_SHAPE_CIRCLE)
	, drawing(false)
{
	control->SetCursor(*wxCROSS_CURSOR);
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolBasicShapeEditor::draw(DC& dc) {
	// highlight the part we are drawing
	if (shape) {
		control.highlightPart(dc, *shape, HIGHLIGHT_BORDER);
	}
}

// ----------------------------------------------------------------------------- : UI

void SymbolBasicShapeEditor::initUI(wxToolBar* tb, wxMenuBar* mb) {
	sides  = new wxSpinCtrl(  tb, ID_SIDES, _("3"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 3, 50, 3);
// 	sidesL = new wxStaticText(tb, ID_SIDES, _(" ") + _LABEL_("sides") + _(": "));
	sides->SetHelpText(_HELP_("sides"));
	sides->SetSize(50, -1);
	tb->AddSeparator();
	tb->AddTool(ID_SHAPE_CIRCLE,	_TOOL_("ellipse"),		load_resource_tool_image(_("circle")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("ellipse"),	_HELP_("ellipse"));
	tb->AddTool(ID_SHAPE_RECTANGLE,	_TOOL_("rectangle"),	load_resource_tool_image(_("rectangle")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("rectangle"),	_HELP_("rectangle"));
	tb->AddTool(ID_SHAPE_POLYGON,	_TOOL_("polygon"),		load_resource_tool_image(_("triangle")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("polygon"),	_HELP_("polygon"));
	tb->AddTool(ID_SHAPE_STAR,		_TOOL_("star"),			load_resource_tool_image(_("star")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("star"),		_HELP_("star"));
// 	tb->AddControl(sidesL);
	tb->AddControl(sides);
	tb->Realize();
	control.SetCursor(*wxCROSS_CURSOR);
}

void SymbolBasicShapeEditor::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->DeleteTool(ID_SHAPE_CIRCLE);
	tb->DeleteTool(ID_SHAPE_RECTANGLE);
	tb->DeleteTool(ID_SHAPE_POLYGON);
	tb->DeleteTool(ID_SHAPE_STAR);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(7); // delete separator
// 	tb->DeleteTool(ID_SIDES_LBEL); // delete sidesL
	tb->DeleteTool(ID_SIDES); // delete sides
	#if wxVERSION_NUMBER < 2600
		delete sides;
		delete sidesL;
	#endif
	stopActions(); // set status text
}

void SymbolBasicShapeEditor::onUpdateUI(wxUpdateUIEvent& ev) {
	if (ev.GetId() >= ID_SHAPE && ev.GetId() < ID_SHAPE_MAX) {
		ev.Check(ev.GetId() == mode);
	} else if (ev.GetId() == ID_SIDES) {
		ev.Enable(mode == ID_SHAPE_POLYGON || mode == ID_SHAPE_STAR);
	} else {
		ev.Enable(false); // we don't know about this item
	}
}

void SymbolBasicShapeEditor::onCommand(int id) {
	if (id >= ID_SHAPE && id < ID_SHAPE_MAX) {
		// change shape mode
		mode = id;
		stopActions();
	}
}

int SymbolBasicShapeEditor::modeToolId() { return ID_MODE_SHAPES; }

// ----------------------------------------------------------------------------- : Mouse events

void SymbolBasicShapeEditor::onLeftDown   (const Vector2D& pos, wxMouseEvent& ev) {
	// Start drawing
	drawing = true;
	start = end = pos;
	SetStatusText(_HELP_("drag to draw shape"));
}

void SymbolBasicShapeEditor::onLeftUp     (const Vector2D& pos, wxMouseEvent& ev) {
	if (drawing && shape) {
		// Finalize the shape
		addAction(new AddSymbolPartAction(*getSymbol(), shape));
		// Select the part
		control.selectPart(shape);
		// no need to clean up, this editor is replaced
		// // Clean up
		// stopActions()
	}
}

void SymbolBasicShapeEditor::onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	// Resize the object
	if (drawing) {
		end = to;
		makeShape(start, end, ev.ControlDown(), settings.symbol_grid_snap, ev.ShiftDown());
		control.Refresh(false);
	}
}

// ----------------------------------------------------------------------------- : Other events

void SymbolBasicShapeEditor::onKeyChange(wxKeyEvent& ev) {
	if (drawing) {
		if (ev.GetKeyCode() == WXK_CONTROL || ev.GetKeyCode() == WXK_SHIFT) {
			// changed constrains
			makeShape(start, end, ev.ControlDown(), settings.symbol_grid_snap, ev.ShiftDown());
			control.Refresh(false);
		} else if (ev.GetKeyCode() == WXK_ESCAPE) {
			// cancel drawing
			stopActions();
		}
	}
}

bool SymbolBasicShapeEditor::isEditing() { return drawing; }

// ----------------------------------------------------------------------------- : Generating shapes

void SymbolBasicShapeEditor::stopActions() {
	shape = SymbolShapeP();
	drawing = false;
	switch (mode) {
		case ID_SHAPE_CIRCLE:
			SetStatusText(_HELP_("draw ellipse"));
			break;
		case ID_SHAPE_RECTANGLE:
			SetStatusText(_HELP_("draw rectangle"));
			break;
		case ID_SHAPE_POLYGON:
			SetStatusText(_HELP_("draw polygon"));
			break;
		case ID_SHAPE_STAR:
			SetStatusText(_HELP_("draw star"));
			break;
	}
	control.Refresh(false);
}

inline double sgn(double d) {
	return d < 0 ? - 1 : 1;
}

void SymbolBasicShapeEditor::makeShape(Vector2D a, Vector2D b, bool constrained, bool snap, bool centered) {
	// snap
	if (snap) {
		a = snap_vector(a, settings.symbol_grid_size);
		b = snap_vector(b, settings.symbol_grid_size);
	}
	// constrain
	Vector2D size = b - a;
	if (constrained) {
		if (fabs(size.x) > fabs(size.y)) {
			size.y = sgn(size.y) * fabs(size.x);
		} else {
			size.x = sgn(size.x) * fabs(size.y);
		}
	}
	// make shape
	if (centered) {
		makeCenteredShape(a, size, constrained);
	} else {
		makeCenteredShape(a + size / 2, size / 2, constrained);
	}
}

// TODO : Move out of this class
void SymbolBasicShapeEditor::makeCenteredShape(const Vector2D& c, Vector2D r, bool constrained) {
	shape = intrusive(new SymbolShape);
	// What shape to make?
	switch (mode) {
		case ID_SHAPE_CIRCLE: {
			// A circle / ellipse
			if (constrained) {
				shape->name = capitalize(_TYPE_("circle"));
			} else {
				shape->name = capitalize(_TYPE_("ellipse"));
			}
			// a circle has 4 control points, the first is: (x+r, y) db(0, kr) da(0, -kr)
			// kr is a magic constant
			const double kr = 0.5522847498f; // = 4/3 * (sqrt(2) - 1)
			shape->points.push_back(intrusive(new ControlPoint(c.x + r.x, c.y,  0, kr  * r.y,  0, -kr * r.y,  LOCK_SIZE)));
			shape->points.push_back(intrusive(new ControlPoint(c.x, c.y - r.y,  kr  * r.x, 0,  -kr * r.x, 0,  LOCK_SIZE)));
			shape->points.push_back(intrusive(new ControlPoint(c.x - r.x, c.y,  0, -kr * r.y,  0, kr  * r.y,  LOCK_SIZE)));
			shape->points.push_back(intrusive(new ControlPoint(c.x, c.y + r.y,  -kr * r.x, 0,  kr  * r.x, 0,  LOCK_SIZE)));
			break;
		} case ID_SHAPE_RECTANGLE: {
			// A rectangle / square
			if (constrained) {
				shape->name = capitalize(_TYPE_("square"));
			} else {
				shape->name = capitalize(_TYPE_("rectangle"));
			}
			// a rectangle just has four corners
			shape->points.push_back(intrusive(new ControlPoint(c.x - r.x, c.y - r.y)));
			shape->points.push_back(intrusive(new ControlPoint(c.x + r.x, c.y - r.y)));
			shape->points.push_back(intrusive(new ControlPoint(c.x + r.x, c.y + r.y)));
			shape->points.push_back(intrusive(new ControlPoint(c.x - r.x, c.y + r.y)));
			break;
		} default: {
			// A polygon or star
			int n = sides->GetValue();  // number of sides
			if (mode == ID_SHAPE_POLYGON) {
				switch (n) {
					case 3:  shape->name = capitalize(_TYPE_("triangle")); break;
					case 4:  shape->name = capitalize(_TYPE_("rhombus")); break;
					case 5:  shape->name = capitalize(_TYPE_("pentagon")); break;
					case 6:  shape->name = capitalize(_TYPE_("hexagon")); break;
					default: shape->name = capitalize(_TYPE_("polygon")); break;
				}
			} else { // star
				shape->name = capitalize(_TYPE_("star"));
			}
			// Example: n == 7
			//         a           a..g = corners
			//      g     b        O    = center
			//     f   O   c       ra   = radius, |Oa|
			//       e   d
			double alpha = 2 * M_PI / n; // internal angle /_aOb
			// angle between point touching side and point on top
			// floor((n+1)/4) == number of sides between these two points
			// beta = /_aOc
			double beta = alpha * ((n+1)/4);
			// define:
			//  width = 2 = |fc|
			//  lb = |ac|
			//  gamma = (pi - beta) / 2
			// equations:
			//  lb * sin(gamma) == 1               (right angled tri /_\ aXc where X is halfway fc)
			//  lb / sin(beta) == ra / sin(gamma)  (law of sines in /_\ abc)
			// solving leads to:
			//  sin(gamma) == cos(beta/2)
			double lb = 1 / cos(beta/2);
			double ra = lb / sin(beta) * cos(beta/2);
			// now we know the center of the polygon:
			double y = c.y + (ra - 1) * r.y;
			if (mode == ID_SHAPE_POLYGON) {
				// we can generate points
				for(int i = 0 ; i < n ; ++i) {
					double theta = alpha * i;
					shape->points.push_back(intrusive(new ControlPoint(
							c.x + ra * r.x * sin(theta),
							y   - ra * r.y * cos(theta)
						)));
				}
			} else {
				// a star is made using a smaller, inverted polygon at the inside
				// points are interleaved
				// rb = radius of smaller polygon
				// lc = length of a side
				double lc = ra * sin(alpha) / cos(alpha/2);
				// ld = length of side skipping one corner
				double delta = alpha * 2;
				double ld = ra * sin(delta) / cos(delta/2);
				// Using symmetry: /_\gab ~ /_\axb where x is intersection
				// gives ratio lc/ld
				// converting back to radius using ra/lb = cos(beta/2) / sin(beta)
				// NOTE: This is only correct for n<=6, but gives acceptable results for higher n
				double rb = (ld - 2 * lc * (lc/ld)) * ra / lb;
				for(int i = 0 ; i < n ; ++i) {
					double theta = alpha * i;
					// from a
					shape->points.push_back(intrusive(new ControlPoint(
							c.x + ra * r.x * sin(theta),
							y   - ra * r.y * cos(theta)
						)));
					// from b
					theta = alpha * (i + 0.5);
					shape->points.push_back(intrusive(new ControlPoint(
							c.x + rb * r.x * sin(theta),
							y   - rb * r.y * cos(theta)
						)));
				}
			}
			break;
		}
	}
}
