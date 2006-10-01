//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/select_editor.hpp>
#include <gui/symbol/window.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <data/action/symbol.hpp>
#include <gfx/gfx.hpp>

// ----------------------------------------------------------------------------- : SymbolSelectEditor

SymbolSelectEditor::SymbolSelectEditor(SymbolControl* control, bool rotate)
	: SymbolEditorBase(control)
	, rotate(rotate)
	, cursorRotate(_("CUR_ROTATE"),  wxBITMAP_TYPE_CUR_RESOURCE)
	, cursorShearX(_("CUR_SHEAR_X"), wxBITMAP_TYPE_CUR_RESOURCE)
	, cursorShearY(_("CUR_SHEAR_Y"), wxBITMAP_TYPE_CUR_RESOURCE)
{
	// Load resource images
	Image rot = loadResourceImage(_("HANDLE_ROTATE"));
	handleRotateTL = wxBitmap(rot);
	handleRotateTR = wxBitmap(rotateImageBy(rot,90));
	handleRotateBR = wxBitmap(rotateImageBy(rot,180));
	handleRotateBL = wxBitmap(rotateImageBy(rot,270));
	Image shear = loadResourceImage(_("HANDLE_SHEAR_X"));
	handleShearX = wxBitmap(shear);
	handleShearY = wxBitmap(rotateImageBy(shear,90));
	handleCenter = wxBitmap(loadResourceImage(_("HANDLE_CENTER")));
	// Make sure all parts have updated bounds
	FOR_EACH(p, getSymbol()->parts) {
		p->calculateBounds();
	}
	resetActions();
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolSelectEditor::draw(DC& dc) {
	// highlight selected parts
	FOR_EACH(p, control.selectedParts) {
		control.highlightPart(dc, *p, HIGHLIGHT_INTERIOR);
	}
	// highlight the part under the cursor
	if (highlightPart) {
		control.highlightPart(dc, *highlightPart, HIGHLIGHT_BORDER);
	}
	// draw handles
	drawHandles(dc);
}

void SymbolSelectEditor::drawHandles(DC& dc) {
	if (control.selectedParts.empty())  return;
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
	tb->AddTool(ID_PART_MERGE,			_("Merge"),		loadResourceImage(_("COMBINE_OR")),			wxNullBitmap, wxITEM_CHECK, _("Merge with shapes below"),			_("Merges this shape with those below it"));
	tb->AddTool(ID_PART_SUBTRACT,		_("Subtract"),	loadResourceImage(_("COMBINE_SUB_DARK")),	wxNullBitmap, wxITEM_CHECK, _("Subtract from shapes below"),		_("Subtracts this shape from shapes below it, leaves only the area in that shape that is not in this shape"));
	tb->AddTool(ID_PART_INTERSECTION,	_("Intersect"),	loadResourceImage(_("COMBINE_AND_DARK")),	wxNullBitmap, wxITEM_CHECK, _("Intersect with shapes below"),		_("Intersects this shape with shapes below it, leaves only the area in both shapes"));
	// note: difference doesn't work (yet)
	tb->AddTool(ID_PART_OVERLAP,		_("Overlap"),	loadResourceImage(_("COMBINE_OVER")),		wxNullBitmap, wxITEM_CHECK, _("Place above other shapes"),			_("Place this shape, and its border above shapes below it"));
	tb->AddTool(ID_PART_BORDER,			_("Border"),	loadResourceImage(_("COMBINE_BORDER")),		wxNullBitmap, wxITEM_CHECK, _("Draw as a border"),					_("Draws this shape as a border"));
	tb->Realize();
}
void SymbolSelectEditor::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->DeleteTool(ID_PART_MERGE);
	tb->DeleteTool(ID_PART_SUBTRACT);
	tb->DeleteTool(ID_PART_INTERSECTION);
	tb->DeleteTool(ID_PART_OVERLAP);
	tb->DeleteTool(ID_PART_BORDER);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(4); // delete separator
}

void SymbolSelectEditor::onUpdateUI(wxUpdateUIEvent& ev) {
	if (ev.GetId() >= ID_PART && ev.GetId() < ID_PART_MAX) {
		if (control.selectedParts.empty()) {
			ev.Check(false);
			ev.Enable(false);
		} else {
			ev.Enable(true);
			bool check = true;
			FOR_EACH(p, control.selectedParts) {
				if (p->combine != ev.GetId() - ID_PART) {
					check = false;
					break;
				}
			}
			ev.Check(check);
		}
	} else if (ev.GetId() == ID_EDIT_DUPLICATE) {
		ev.Enable(!control.selectedParts.empty());
	} else {
		ev.Enable(false); // we don't know about this item
	}
}

void SymbolSelectEditor::onCommand(int id) {
	if (id >= ID_PART && id < ID_PART_MAX) {
		// change combine mode
		getSymbol()->actions.add(new CombiningModeAction(
				control.selectedParts,
				static_cast<SymbolPartCombine>(id - ID_PART)
			));
		control.Refresh(false);
	} else if (id == ID_EDIT_DUPLICATE && !isEditing()) {
		// duplicate selection, not when dragging
		DuplicateSymbolPartsAction* action = new DuplicateSymbolPartsAction(
				*getSymbol(), control.selectedParts
			);
		getSymbol()->actions.add(action);
		control.Refresh(false);
	}
}

int SymbolSelectEditor::modeToolId() {
	return  rotate ? ID_MODE_ROTATE : ID_MODE_SELECT;
}

// ----------------------------------------------------------------------------- : Mouse Events

void SymbolSelectEditor::onLeftDown  (const Vector2D& pos, wxMouseEvent& ev) {
}

void SymbolSelectEditor::onLeftUp    (const Vector2D& pos, wxMouseEvent& ev) {
	if (isEditing()) {
		// stop editing
		resetActions();
	} else {
		// mouse not moved, change selection
		// Are we on a handle?
		int dx, dy;
		if (onAnyHandle(pos, &dx, &dy)) return; // don't change the selection
		// Select the part under the cursor
		SymbolPartP part = findPart(pos);
		if (part) {
			if (ev.ShiftDown()) {
				// toggle selection
				set<SymbolPartP>::iterator it = control.selectedParts.find(part);
				if (it != control.selectedParts.end()) {
					control.selectedParts.erase(it);
				} else {
					control.selectedParts.insert(part);
				}
			} else {
				if (control.selectedParts.find(part) != control.selectedParts.end()) {
					// already selected, don't change selection
					// instead switch between rotate and resize mode
					rotate = !rotate;
				} else {
					// select the part under the cursor
					control.selectedParts.clear();
					control.selectedParts.insert(part);
				}
			}
		} else if (!ev.ShiftDown()) {
			// select nothing
			control.selectedParts.clear();
		}
		// selection has changed
		updateBoundingBox();
		control.signalSelectionChange();
	}
	control.Refresh(false);
}

void SymbolSelectEditor::onLeftDClick(const Vector2D& pos, wxMouseEvent& ev) {
	// start editing the points of the clicked part
	highlightPart = findPart(pos);
	if (highlightPart) {
		control.activatePart(highlightPart);
	}
}

void SymbolSelectEditor::onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& e) {
	// can we highlight a part?
	highlightPart = findPart(to);
	// are we on a handle?
	int dx, dy;
	if (!control.selectedParts.empty() && onAnyHandle(to, &dx, &dy)) {
		// we are on a handle, don't highlight
		highlightPart = SymbolPartP();
		if (rotate) {
			// shear or rotating?
			if (dx == 0 || dy == 0) {
				SetStatusText(String(_("Drag to shear selected shape")) + (control.selectedParts.size() > 1 ? _("s") : _("")));
				control.SetCursor(dx == 0 ? cursorShearX : cursorShearY);
			} else {
				SetStatusText(String(_("Drag to rotate selected shape")) + (control.selectedParts.size() > 1 ? _("s") : _("")) + _(", Ctrl constrains angle to multiples of 15 degrees"));
				control.SetCursor(cursorRotate);
			}
		} else {
			SetStatusText(String(_("Drag to resize selected shape")) + (control.selectedParts.size() > 1 ? _("s") : _("")) + _(", Ctrl constrains size"));
			// what cursor to use?
			if      (dx ==  dy) control.SetCursor(wxCURSOR_SIZENWSE);
			else if (dx == -dy) control.SetCursor(wxCURSOR_SIZENESW);
			else if (dx == 0)   control.SetCursor(wxCURSOR_SIZENS);
			else if (dy == 0)   control.SetCursor(wxCURSOR_SIZEWE);
		}
	} else {
		if (highlightPart) {
			SetStatusText(_("Click to select shape, drag to move shape, double click to edit shape"));
		} else {
			SetStatusText(_(""));
		}
		control.SetCursor(*wxSTANDARD_CURSOR);
	}
	control.Refresh(false);
}

void SymbolSelectEditor::onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	if (control.selectedParts.empty()) return;
	if (!isEditing()) {
		// we don't have an action yet, determine what to do
		// note: base it on the from position, which is the position where dragging started
		if (onAnyHandle(from, &scaleX, &scaleY)) {
			if (rotate) {
				if (scaleX == 0 || scaleY == 0) {
					// shear, center/fixed point on the opposite side
					shearAction = new SymbolPartShearAction(control.selectedParts, handlePos(-scaleX, -scaleY));
					getSymbol()->actions.add(shearAction);
				} else {
					// rotate	
					rotateAction = new SymbolPartRotateAction(control.selectedParts, center);
					getSymbol()->actions.add(rotateAction);
					startAngle = angleTo(to);
				}
			} else {
				// we are on a handle; start scaling
				scaleAction = new SymbolPartScaleAction(control.selectedParts, scaleX, scaleY);
				getSymbol()->actions.add(scaleAction);
			}
		} else {
			// move
			moveAction = new SymbolPartMoveAction(control.selectedParts);
			getSymbol()->actions.add(moveAction);
		}
	}
	
	// now update the action
	if (moveAction) {
		// move the selected parts
		moveAction->constrain =	ev.ControlDown();
		moveAction->move(to - from);
	} else if (scaleAction) {
		// scale the selected parts
		Vector2D delta = to-from;
		Vector2D dMin, dMax;
		if (scaleX == -1) dMin.x = delta.x;
		if (scaleX ==  1) dMax.x = delta.x;
		if (scaleY == -1) dMin.y = delta.y;
		if (scaleY ==  1) dMax.y = delta.y;
		scaleAction->constrain = ev.ControlDown();
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
		shearAction->constrain = ev.ControlDown();
		shearAction->move(delta);
	}
	control.Refresh(false);
}

// ----------------------------------------------------------------------------- : Key Events

void SymbolSelectEditor::onKeyChange (wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_CONTROL) {
		// changed constrains
		if (moveAction) {
			moveAction->constrain = ev.ControlDown();
			moveAction->move(Vector2D()); // apply constrains
			control.Refresh(false);
		} else if (scaleAction) {
			// only allow constrained scaling in diagonal direction
			scaleAction->constrain = ev.ControlDown();
			scaleAction->update(); // apply constrains
			control.Refresh(false);
		} else if (rotateAction) {
			rotateAction->constrain = ev.ControlDown();
			rotateAction->rotateBy(0); // apply constrains
			control.Refresh(false);
		}
	}
}
void SymbolSelectEditor::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_DELETE) {
		// delete selected parts
		getSymbol()->actions.add(new RemoveSymbolPartsAction(*getSymbol(), control.selectedParts));
		control.selectedParts.clear();
		resetActions();
		control.Refresh(false);
	} else {
		ev.Skip();
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
	for (int dx = -1 ; dx < 1 ; ++dx) {
		for (int dy = -1 ; dy < 1 ; ++dy) {
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


SymbolPartP SymbolSelectEditor::findPart(const Vector2D& pos) {
	FOR_EACH(p, getSymbol()->parts) {
		if (pointInPart(pos, *p)) return p;
	}
	return SymbolPartP();
}

void SymbolSelectEditor::updateBoundingBox() {
	// Find min and max coordinates
	minV =  Vector2D::infinity();
	maxV = -Vector2D::infinity();
	FOR_EACH(p, control.selectedParts) {
		minV = piecewise_min(minV, p->minPos);
		maxV = piecewise_max(maxV, p->maxPos);
	}
	// Find rotation center
	center = Vector2D(0,0);
	FOR_EACH(p, control.selectedParts) {
		Vector2D size = p->maxPos - p->minPos;
		size = size.mul(p->rotationCenter);
		center += p->minPos + size;
	}
	center /= control.selectedParts.size();
}

void SymbolSelectEditor::resetActions() {
	moveAction   = nullptr;
	scaleAction  = nullptr;
	rotateAction = nullptr;
	shearAction  = nullptr;
}
