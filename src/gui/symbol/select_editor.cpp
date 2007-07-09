//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/select_editor.hpp>
#include <gui/symbol/window.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <data/action/symbol.hpp>
#include <data/settings.hpp>
#include <util/error.hpp>
#include <gfx/gfx.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : SymbolSelectEditor

SymbolSelectEditor::SymbolSelectEditor(SymbolControl* control, bool rotate)
	: SymbolEditorBase(control)
	, click_mode(CLICK_NONE)
	, rotate(rotate)
	, cursorRotate(load_resource_cursor(_("rotate")))
	, cursorShearX(load_resource_cursor(_("shear_x")))
	, cursorShearY(load_resource_cursor(_("shear_y")))
{
	// Load resource images
	Image rot = load_resource_image(_("handle_rotate"));
	handleRotateTL = wxBitmap(rot);
	handleRotateTR = wxBitmap(rotate_image(rot,90));
	handleRotateBR = wxBitmap(rotate_image(rot,180));
	handleRotateBL = wxBitmap(rotate_image(rot,270));
	Image shear = load_resource_image(_("handle_shear_x"));
	handleShearX = wxBitmap(shear);
	handleShearY = wxBitmap(rotate_image(shear,90));
	handleCenter = wxBitmap(load_resource_image(_("handle_center")));
	// Make sure all parts have updated bounds
	getSymbol()->calculateBounds();
	resetActions();
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolSelectEditor::draw(DC& dc) {
	// highlight selected parts
	FOR_EACH(p, control.selected_parts.get()) {
		control.highlightPart(dc, *p, HIGHLIGHT_INTERIOR);
	}
	// highlight the part under the cursor
	if (highlightPart) {
		control.highlightPart(dc, *highlightPart, HIGHLIGHT_BORDER);
	}
	if (click_mode == CLICK_RECT) {
		// draw selection rectangle
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxPen(*wxBLUE,1,wxDOT));
		RealRect rect = control.rotation.tr(RealRect(selection_rect_a, RealSize(selection_rect_b - selection_rect_a)));
		dc.DrawRectangle(rect);
	} else {
		// draw handles
		drawHandles(dc);
	}
}

void SymbolSelectEditor::drawHandles(DC& dc) {
	if (control.selected_parts.empty()) return;
	if (rotateAction) return; // not when rotating
	updateBoundingBox();
	// Draw handles on all sides
	for (int dx = -1 ; dx <= 1 ; ++dx) {
		for (int dy = -1 ; dy <= 1 ; ++dy) {
			if (dx != 0 || dy != 0) {
				// no handle in the center
				drawHandle(dc, dx, dy);
			}
		}
	}
	// Draw rotation center?
	if (rotate) {
		drawRotationCenter(dc, center);
	}
}

void SymbolSelectEditor::drawHandle(DC& dc, int dx, int dy) {
	wxPoint p = control.rotation.tr(handlePos(dx, dy));
	p.x += 4 * dx;
	p.y += 4 * dy;
	if (rotate) {
		// rotate or shear handle
		if (dx == 0) dc.DrawBitmap(handleShearX, p.x - 10, p.y - 3 - (dy < 0 ? 1 : 0));
		if (dy == 0) dc.DrawBitmap(handleShearY, p.x - 3 - (dx < 0 ? 1 : 0), p.y - 10);
		else {
			// rotate
			if (dx == -1 && dy == -1) dc.DrawBitmap(handleRotateTL, p.x - 5,  p.y - 5);
			if (dx == -1 && dy ==  1) dc.DrawBitmap(handleRotateTR, p.x - 5,  p.y - 11);
			if (dx ==  1 && dy == -1) dc.DrawBitmap(handleRotateBL, p.x - 11, p.y - 5);
			if (dx ==  1 && dy ==  1) dc.DrawBitmap(handleRotateBR, p.x - 11, p.y - 11);
		}
	} else {
		// resize handle
		dc.SetBrush(*wxBLUE_BRUSH);
		dc.SetPen(  *wxWHITE_PEN);
		dc.DrawRectangle(p.x - 3, p.y - 3, 6, 6);
	}
}

void SymbolSelectEditor::drawRotationCenter(DC& dc, const Vector2D& pos) {
	wxPoint p = control.rotation.tr(pos);
	dc.DrawBitmap(handleCenter, p.x - 9, p.y - 9);
}

// ----------------------------------------------------------------------------- : UI

void SymbolSelectEditor::initUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->AddSeparator();
	tb->AddTool(ID_SYMBOL_COMBINE_MERGE,        _TOOL_("merge"),      load_resource_image(_("combine_or")),       wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("merge"),      _HELP_("merge"));
	tb->AddTool(ID_SYMBOL_COMBINE_SUBTRACT,     _TOOL_("subtract"),   load_resource_image(_("combine_sub_dark")), wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("subtract"),   _HELP_("subtract"));
	tb->AddTool(ID_SYMBOL_COMBINE_INTERSECTION, _TOOL_("intersect"),  load_resource_image(_("combine_and_dark")), wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("intersect"),  _HELP_("intersect"));
	tb->AddTool(ID_SYMBOL_COMBINE_DIFFERENCE,   _TOOL_("difference"), load_resource_image(_("combine_xor")),      wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("difference"), _HELP_("difference"));
	tb->AddTool(ID_SYMBOL_COMBINE_OVERLAP,      _TOOL_("overlap"),    load_resource_image(_("combine_over")),     wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("overlap"),    _HELP_("overlap"));
	tb->AddTool(ID_SYMBOL_COMBINE_BORDER,       _TOOL_("border"),     load_resource_image(_("combine_border")),   wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("border"),     _HELP_("border"));
	tb->Realize();
}
void SymbolSelectEditor::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->DeleteTool(ID_SYMBOL_COMBINE_MERGE);
	tb->DeleteTool(ID_SYMBOL_COMBINE_SUBTRACT);
	tb->DeleteTool(ID_SYMBOL_COMBINE_INTERSECTION);
	tb->DeleteTool(ID_SYMBOL_COMBINE_DIFFERENCE);
	tb->DeleteTool(ID_SYMBOL_COMBINE_OVERLAP);
	tb->DeleteTool(ID_SYMBOL_COMBINE_BORDER);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(7); // delete separator
}

void SymbolSelectEditor::onUpdateUI(wxUpdateUIEvent& ev) {
	if (ev.GetId() >= ID_SYMBOL_COMBINE && ev.GetId() < ID_SYMBOL_COMBINE_MAX) {
		bool enable = false;
		bool check = true;
		FOR_EACH(p, control.selected_parts.get()) {
			if (SymbolShape* s = p->isSymbolShape()) {
				enable = true;
				if (s->combine != ev.GetId() - ID_SYMBOL_COMBINE) {
					check = false;
					break;
				}
			} // disable when symmetries are selected?
		}
		ev.Enable(enable);
		ev.Check(enable && check);
	} else if (ev.GetId() == ID_EDIT_DUPLICATE) {
		ev.Enable(!control.selected_parts.empty());
	} else if (ev.GetId() == ID_EDIT_GROUP) {
		ev.Enable(control.selected_parts.size() >= 2);
	} else if (ev.GetId() == ID_EDIT_UNGROUP) {
		// is a group selected
		FOR_EACH(p, control.selected_parts.get()) {
			if (p->isSymbolGroup() && !p->isSymbolSymmetry()) {
				ev.Enable(true);
				return;
			}
		}
		ev.Enable(false);
	} else {
		ev.Enable(false); // we don't know about this item
	}
}

void SymbolSelectEditor::onCommand(int id) {
	if (id >= ID_SYMBOL_COMBINE && id < ID_SYMBOL_COMBINE_MAX) {
		// change combine mode
		getSymbol()->actions.add(new CombiningModeAction(
				control.selected_parts.get(),
				static_cast<SymbolShapeCombine>(id - ID_SYMBOL_COMBINE)
			));
		control.Refresh(false);
	} else if (id == ID_EDIT_DUPLICATE && !isEditing()) {
		// duplicate selection, not when dragging
		getSymbol()->actions.add(new DuplicateSymbolPartsAction(*getSymbol(), control.selected_parts.get()));
		control.Refresh(false);
	} else if (id == ID_EDIT_GROUP && !isEditing()) {
		// group selection, not when dragging
		getSymbol()->actions.add(new GroupSymbolPartsAction(*getSymbol(), control.selected_parts.get(), new_intrusive<SymbolGroup>()));
		control.Refresh(false);
	} else if (id == ID_EDIT_UNGROUP && !isEditing()) {
		// ungroup selection, not when dragging
		getSymbol()->actions.add(new UngroupSymbolPartsAction(*getSymbol(), control.selected_parts.get()));
		control.Refresh(false);
	}
}

int SymbolSelectEditor::modeToolId() {
	return  rotate ? ID_MODE_ROTATE : ID_MODE_SELECT;
}

// ----------------------------------------------------------------------------- : Mouse Events

void SymbolSelectEditor::onLeftDown  (const Vector2D& pos, wxMouseEvent& ev) {
	// change selection
	// Are we on a handle?
	int dx, dy;
	if (onAnyHandle(pos, &dx, &dy)) {
		click_mode = CLICK_HANDLE;
		return; // don't change the selection
	}
	// Select the part under the cursor
	SymbolPartP part = control.selected_parts.find(pos);
	if (part) {
		click_mode = control.selected_parts.select(part, ev.ShiftDown() ? SELECT_TOGGLE : SELECT_IF_OUTSIDE)
		           ? (ev.ShiftDown() ? CLICK_NONE : CLICK_MOVE)
		           : CLICK_TOGGLE;
	} else {
		// selection rectangle
		click_mode = CLICK_RECT;
		selection_rect_a = selection_rect_b = pos;
		if (!ev.ShiftDown()) {
			// select nothing
			control.selected_parts.clear();
		}
	}
	// selection has changed
	updateBoundingBox();
	control.signalSelectionChange();
	control.Refresh(false);
}

void SymbolSelectEditor::onLeftUp    (const Vector2D& pos, wxMouseEvent& ev) {
	if (isEditing()) {
		// stop editing
		resetActions();
	} else {
		// mouse not moved -> change selection
		if (click_mode == CLICK_TOGGLE) {
			// switch between rotate and resize mode
			rotate = !rotate;
		}
	}
	click_mode = CLICK_NONE;
	control.Refresh(false);
}

void SymbolSelectEditor::onLeftDClick(const Vector2D& pos, wxMouseEvent& ev) {
	// start editing the points of the clicked part
	highlightPart = control.selected_parts.find(pos);
	if (highlightPart) {
		control.activatePart(highlightPart);
	}
}

void SymbolSelectEditor::onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	// can we highlight a part?
	highlightPart = control.selected_parts.find(to);
	// are we on a handle?
	int dx, dy;
	if (!control.selected_parts.empty() && onAnyHandle(to, &dx, &dy)) {
		// we are on a handle, don't highlight
		highlightPart = SymbolPartP();
		String shapes = control.selected_parts.size() > 1 ? _TYPE_("shapes") : _TYPE_("shape");
		if (rotate) {
			// shear or rotating?
			if (dx == 0 || dy == 0) {
				SetStatusText(_HELP_1_("drag to shear", shapes));
				control.SetCursor(dx == 0 ? cursorShearX : cursorShearY);
			} else {
				SetStatusText(_HELP_1_("drag to rotate", shapes));
				control.SetCursor(cursorRotate);
			}
		} else {
			SetStatusText(_HELP_1_("drag to resize", shapes));
			// what cursor to use?
			if      (dx ==  dy) control.SetCursor(wxCURSOR_SIZENWSE);
			else if (dx == -dy) control.SetCursor(wxCURSOR_SIZENESW);
			else if (dx == 0)   control.SetCursor(wxCURSOR_SIZENS);
			else if (dy == 0)   control.SetCursor(wxCURSOR_SIZEWE);
		}
	} else {
		if (highlightPart) {
			SetStatusText(_HELP_("click to select shape"));
		} else {
			SetStatusText(_(""));
		}
		control.SetCursor(*wxSTANDARD_CURSOR);
	}
	control.Refresh(false);
}

template <typename Event> int snap(Event& ev) {
	return settings.symbol_grid_snap != ev.ShiftDown() ? settings.symbol_grid_size : 0; // shift toggles snap
}

void SymbolSelectEditor::onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	if (click_mode == CLICK_NONE) return;
	if (control.selected_parts.empty()) return;
	if (click_mode == CLICK_RECT) {
		// rectangle
		control.selected_parts.selectRect(selection_rect_a, selection_rect_b, to, SELECT_TOGGLE);
		selection_rect_b = to;
		control.Refresh(false);
	}
	if (!isEditing()) {
		// we don't have an action yet, determine what to do
		// note: base it on the from position, which is the position where dragging started
		if (onAnyHandle(from, &scaleX, &scaleY)) {
			click_mode = CLICK_HANDLE;
			if (rotate) {
				if (scaleX == 0 || scaleY == 0) {
					// shear, center/fixed point on the opposite side
					shearAction = new SymbolPartShearAction(control.selected_parts.get(), handlePos(-scaleX, -scaleY));
					getSymbol()->actions.add(shearAction);
				} else {
					// rotate	
					rotateAction = new SymbolPartRotateAction(control.selected_parts.get(), center);
					getSymbol()->actions.add(rotateAction);
					startAngle = angleTo(to);
				}
			} else {
				// we are on a handle; start scaling
				scaleAction = new SymbolPartScaleAction(control.selected_parts.get(), scaleX, scaleY);
				getSymbol()->actions.add(scaleAction);
			}
		} else {
			// move
			click_mode = CLICK_MOVE;
			moveAction = new SymbolPartMoveAction(control.selected_parts.get());
			getSymbol()->actions.add(moveAction);
		}
	}
	
	// now update the action
	if (moveAction) {
		// move the selected parts
		moveAction->constrain =	ev.ControlDown();
		moveAction->snap      = snap(ev);
		moveAction->move(to - from);
	} else if (scaleAction) {
		// scale the selected parts
		Vector2D delta = to-from;
		Vector2D dMin, dMax;
		if (scaleX == -1) dMin.x = delta.x;
		if (scaleX ==  1) dMax.x = delta.x;
		if (scaleY == -1) dMin.y = delta.y;
		if (scaleY ==  1) dMax.y = delta.y;
//		scaleAction->constrain = ev.ControlDown();
		scaleAction->constrain = true; // always constrain diagonal scaling
		scaleAction->snap      = snap(ev);
		scaleAction->move(dMin,	dMax);
	} else if (rotateAction) {
		// rotate the selected parts
		double angle = angleTo(to);
		rotateAction->constrain	= ev.ControlDown();
		rotateAction->rotateTo(startAngle - angle);
	} else if (shearAction) {
		// shear the selected parts
		Vector2D delta = to-from;
		delta = delta.mul(Vector2D(scaleY, scaleX));
		delta = delta.div(maxV - minV);
//		shearAction->constrain = ev.ControlDown();
		shearAction->snap      = snap(ev);
		shearAction->move(delta);
	}
	control.Refresh(false);
}

// ----------------------------------------------------------------------------- : Key Events

void SymbolSelectEditor::onKeyChange (wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_CONTROL || ev.GetKeyCode() == WXK_SHIFT) {
		// changed constrains
		if (moveAction) {
			moveAction->constrain = ev.ControlDown();
			moveAction->snap      = snap(ev);
			moveAction->move(Vector2D()); // apply constrains
			control.Refresh(false);
		} else if (scaleAction) {
			// only allow constrained scaling in diagonal direction
//			scaleAction->constrain = ev.ControlDown();
			scaleAction->constrain = true; // always constrain diagonal scaling
			scaleAction->snap      = snap(ev);
			scaleAction->update(); // apply constrains
			control.Refresh(false);
		} else if (rotateAction) {
			rotateAction->constrain = ev.ControlDown();
			rotateAction->rotateBy(0); // apply constrains
			control.Refresh(false);
		} else if (shearAction) {
			shearAction->snap      = snap(ev);
			shearAction->move(Vector2D()); // apply constrains
			control.Refresh(false);
		}
	}
}
void SymbolSelectEditor::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_DELETE) {
		// delete selected parts
		getSymbol()->actions.add(new RemoveSymbolPartsAction(*getSymbol(), control.selected_parts.get()));
		if (control.selected_parts.selected(highlightPart)) highlightPart = SymbolPartP(); // deleted it
		control.selected_parts.clear();
		resetActions();
		control.Refresh(false);
	} else {
		// move selection using arrow keys
		double step = 1.0 / settings.symbol_grid_size;
		Vector2D delta;
		if      (ev.GetKeyCode() == WXK_LEFT)  delta = Vector2D(-step, 0);
		else if (ev.GetKeyCode() == WXK_RIGHT) delta = Vector2D( step, 0);
		else if (ev.GetKeyCode() == WXK_UP)    delta = Vector2D(0, -step);
		else if (ev.GetKeyCode() == WXK_DOWN)  delta = Vector2D(0,  step);
		else {
			ev.Skip();
			return;
		}
		getSymbol()->actions.add(new SymbolPartMoveAction(control.selected_parts.get(), delta));
	}
}

bool SymbolSelectEditor::isEditing() {
	return moveAction || scaleAction || rotateAction || shearAction;
}

// ----------------------------------------------------------------------------- : Other

Vector2D SymbolSelectEditor::handlePos(int dx, int dy) {
	return Vector2D(
		0.5 * (maxV.x + minV.x + dx * (maxV.x - minV.x)),
		0.5 * (maxV.y + minV.y + dy * (maxV.y - minV.y))
	);
}

bool SymbolSelectEditor::onHandle(const Vector2D& mpos, int dx, int dy) {
	wxPoint p  = control.rotation.tr(handlePos(dx, dy));
	wxPoint mp = control.rotation.tr(mpos);
	p.x = p.x + 4 * dx;
	p.y = p.y + 4 * dy;
	return mp.x >= p.x - 4 && mp.x < p.x + 4 &&
	       mp.y >= p.y - 4 && mp.y < p.y + 4;
}
bool SymbolSelectEditor::onAnyHandle(const Vector2D& mpos, int* dxOut, int* dyOut) {
	for (int dx = -1 ; dx <= 1 ; ++dx) {
		for (int dy = -1 ; dy <= 1 ; ++dy) {
			if ((dx != 0 || dy != 0) && onHandle(mpos, dx, dy)) { // (0,0) == center, not a handle
				*dxOut = dx;
				*dyOut = dy;
				return true;
			}
		}
	}
	return false;
}

double SymbolSelectEditor::angleTo(const Vector2D& pos) {
	return atan2(center.x - pos.x, center.y - pos.y);
}

void SymbolSelectEditor::updateBoundingBox() {
	// Find min and max coordinates
	minV =  Vector2D::infinity();
	maxV = -Vector2D::infinity();
	FOR_EACH(p, control.selected_parts.get()) {
		minV = piecewise_min(minV, p->min_pos);
		maxV = piecewise_max(maxV, p->max_pos);
	}
/*	// Find rotation center
	center = Vector2D(0,0);
	FOR_EACH(p, control.selected_parts) {
		Vector2D size = p->max_pos - p->min_pos;
		size = size.mul(p->rotation_center);
		center += p->min_pos + size;
	}
	center /= control.selected_parts.size();
*/
	center = (minV + maxV) / 2;
}

void SymbolSelectEditor::resetActions() {
	moveAction   = nullptr;
	scaleAction  = nullptr;
	rotateAction = nullptr;
	shearAction  = nullptr;
}
