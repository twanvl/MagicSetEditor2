//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/point_editor.hpp>
#include <gui/symbol/window.hpp>
#include <gfx/bezier.hpp>
#include <data/action/symbol_part.hpp>
#include <util/window_id.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(ControlPointP);

// ----------------------------------------------------------------------------- : SymbolPointEditor

SymbolPointEditor::SymbolPointEditor(SymbolControl* control, const SymbolPartP& part)
	: SymbolEditorBase(control)
	, part(part)
	, selection(SELECTED_NONE)
	, hovering(SELECTED_NONE)
	// Load gui stock
	, pointSelect(_("CUR_POINT"),      wxBITMAP_TYPE_CUR_RESOURCE)
	, pointAdd   (_("CUR_POINT_ADD"),  wxBITMAP_TYPE_CUR_RESOURCE)
	, pointCurve (_("CUR_POINT_CURVE"),wxBITMAP_TYPE_CUR_RESOURCE)
	, pointMove  (_("CUR_POINT_MOVE"), wxBITMAP_TYPE_CUR_RESOURCE)
{
	resetActions();
//	// fix pen joins
//	penHandleHover.join = wxJOIN_MITER;
//	penMainHover.join   = wxJOIN_MITER;
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolPointEditor::draw(DC& dc) {
	// highlight the part
	control.highlightPart(dc, *part, HIGHLIGHT_BORDER);
	// handles etc.
	if (hovering == SELECTED_LINE) {
		drawHoveredLine(dc);
	}
	drawHandles(dc);
	if (hovering == SELECTED_NEW_POINT) {
		drawNewPoint(dc);
	}
}

void SymbolPointEditor::drawHoveredLine(DC& dc) {
	BezierCurve c(*hoverLine1, *hoverLine2);
	wxPoint prevPoint = control.rotation.tr(hoverLine1->pos);
	for(int i = 1 ; i <= 100 ; ++i) {
		// Draw 100 segments of the curve
		double t = double(i)/100.0f;
		wxPoint curPoint = control.rotation.tr(c.pointAt(t));
		double selectPercent = 1.0 - 1.2 * sqrt(fabs(hoverLineT-t)); // amount to color
		if (selectPercent > 0) {
			// gradient color
			Color color(
				col(300 - 300 * selectPercent),
				col(300 * selectPercent),
				col(0)
			);
			dc.SetPen(wxPen(color, 3));
			dc.DrawLine(prevPoint, curPoint);
		}
		prevPoint = curPoint;
	}
}

void SymbolPointEditor::drawHandles(DC& dc) {
	dc.SetPen(Color(0,0,128));
	dc.SetBrush(Color(128,128,255));
	for (int i = 0 ; (size_t)i < part->points.size() ; ++i) {
		// determine which handles to draw
		bool selected  = pointSelected(*part->getPoint(i));
		bool selBefore = selected || pointSelected(*part->getPoint(i-1));
		bool selAfter  = selected || pointSelected(*part->getPoint(i+1));
		// and draw them
		drawControlPoint(dc, *part->getPoint(i), selBefore, selAfter);
	}
}

void SymbolPointEditor::drawNewPoint(DC& dc) {
	dc.SetPen(*wxGREEN_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	wxPoint p = control.rotation.tr(newPoint);
	drawHandleBox(dc, p.x, p.y, true);
}

void SymbolPointEditor::drawControlPoint(DC& dc, const ControlPoint& p, bool drawHandleBefore, bool drawHandleAfter) {
	// Position
	wxPoint p0 = control.rotation.tr(p.pos);
	// Sub handles
	if (drawHandleBefore || drawHandleAfter) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		// Before handle
		if (drawHandleBefore && p.segmentBefore == SEGMENT_CURVE) {
			wxPoint p1 = control.rotation.tr(p.pos + p.deltaBefore);
			dc.SetPen(handlePen(PEN_LINE, p.lock));
			dc.DrawLine(p0.x, p0.y, p1.x, p1.y);
			dc.SetPen(handlePen(handleHovered(p, HANDLE_BEFORE) ? PEN_HOVER : PEN_NORMAL, p.lock));
			drawHandleCircle(dc, p1.x, p1.y);
		}
		// After handle
		if (drawHandleAfter && p.segmentAfter == SEGMENT_CURVE) {
			wxPoint p1 = control.rotation.tr(p.pos + p.deltaAfter);
			dc.SetPen(handlePen(PEN_LINE, p.lock));
			dc.DrawLine(p0.x, p0.y, p1.x, p1.y);
			dc.SetPen(handlePen(handleHovered(p, HANDLE_AFTER) ? PEN_HOVER : PEN_NORMAL, p.lock));
			drawHandleCircle(dc, p1.x, p1.y);
		}
	}
	// Main handle
	// last, so it draws over lines to handles
	bool selected = pointSelected(p);
	wxPen hp(*wxBLACK, pointHovered(p) ? 2 : 1);
	hp.SetJoin(wxJOIN_MITER);
	dc.SetPen(hp);
	dc.SetBrush(selected ? *wxGREEN_BRUSH : *wxTRANSPARENT_BRUSH);
	drawHandleBox(dc, p0.x, p0.y, selected);
}

void SymbolPointEditor::drawHandleBox(DC& dc, UInt px, UInt py, bool active) {
	dc.DrawRectangle(px - 3, py - 3, 7, 7);
	if (!active) {
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawRectangle(px - 2, py - 2, 5, 5);
	}
}
void SymbolPointEditor::drawHandleCircle(DC& dc, UInt px, UInt py) {
	dc.DrawCircle(px, py, 4);
}

wxPen SymbolPointEditor::handlePen(WhichPen p, LockMode lock) {
	Color col;
	if (lock == LOCK_FREE) col = Color(100, 100, 255);
	if (lock == LOCK_DIR)  col = Color(153, 0,   204);
	if (lock == LOCK_SIZE) col = Color(204, 50,  50);
	switch(p) {
		case PEN_NORMAL: return wxPen(col);
		case PEN_HOVER:  return wxPen(col, 2);
		case PEN_LINE:   return wxPen(col, 1, wxDOT);
		default:         throw InternalError(_("SymbolPointEditor::handlePen"));
	}
}

// ----------------------------------------------------------------------------- : UI

void SymbolPointEditor::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// Initialize toolbar
	tb->AddSeparator();
	tb->AddTool(ID_SEGMENT_LINE,	_("Line"),		Bitmap(_("TOOL_LINE")),			wxNullBitmap, wxITEM_CHECK, _("To straigt line"),		_("Makes the selected line straight"));
	tb->AddTool(ID_SEGMENT_CURVE,	_("Curve"),		Bitmap(_("TOOL_CURVE")),		wxNullBitmap, wxITEM_CHECK, _("To curve"),				_("Makes the selected line curved"));
	tb->AddSeparator();
	tb->AddTool(ID_LOCK_FREE,		_("Free"),		Bitmap(_("TOOL_LOCK_FREE")),	wxNullBitmap, wxITEM_CHECK, _("Unlock node"),			_("Allows the two control points on the node to be moved freely"));
	tb->AddTool(ID_LOCK_DIR,		_("Smooth"),	Bitmap(_("TOOL_LOCK_DIR")),		wxNullBitmap, wxITEM_CHECK, _("Make node smooth"),		_("Makes the selected node smooth by placing the two control points opposite each other"));
	tb->AddTool(ID_LOCK_SIZE,		_("Symmetric"),	Bitmap(_("TOOL_LOCK_SIZE")),	wxNullBitmap, wxITEM_CHECK, _("Make node symmetric"),	_("Makes the selected node symetric"));
	tb->Realize();
	// TODO : menu bar
	//mb->Insert(2, curveMenu, _("&Curve"))
}

void SymbolPointEditor::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->DeleteTool(ID_SEGMENT_LINE);
	tb->DeleteTool(ID_SEGMENT_CURVE);
	tb->DeleteTool(ID_LOCK_FREE);
	tb->DeleteTool(ID_LOCK_DIR);
	tb->DeleteTool(ID_LOCK_SIZE);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(4); // delete separator
	tb->DeleteToolByPos(4); // delete separator
	// TODO : menu bar
	//mb->Remove(2)
}

void SymbolPointEditor::onUpdateUI(wxUpdateUIEvent& ev) {
	// enable
	bool enabled = false, checked = false;
	switch (ev.GetId()) {
		case ID_SEGMENT_LINE: case ID_SEGMENT_CURVE:
			enabled = selection == SELECTED_LINE;
			break;
		case ID_LOCK_FREE: case ID_LOCK_DIR: case ID_LOCK_SIZE:
			enabled = selection == SELECTED_POINTS &&
				        selectedPoints.size() == 1 &&
				        (*selectedPoints.begin())->segmentBefore == SEGMENT_CURVE &&
				        (*selectedPoints.begin())->segmentAfter  == SEGMENT_CURVE;
			break;
		default:
			ev.Enable(false); // we don't know this item
			return;
	}
	// check
	if (enabled) {
		switch (ev.GetId()) {
			case ID_SEGMENT_LINE: case ID_SEGMENT_CURVE:
				checked = selectedLine1->segmentAfter == ev.GetId() - ID_SEGMENT;
				break;
			case ID_LOCK_FREE: case ID_LOCK_DIR: case ID_LOCK_SIZE:
				checked = (*selectedPoints.begin())->lock == ev.GetId() - ID_LOCK;
				break;
		}
	}
	
	ev.Enable(enabled);
	ev.Check(checked);
}

void SymbolPointEditor::onCommand(int id) {
	switch (id) {
		case ID_SEGMENT_LINE: case ID_SEGMENT_CURVE:
			onChangeSegment( static_cast<SegmentMode>(id - ID_SEGMENT) );
		case ID_LOCK_FREE: case ID_LOCK_DIR: case ID_LOCK_SIZE:
			onChangeLock( static_cast<LockMode>(id - ID_LOCK) );
	}
}


int SymbolPointEditor::modeToolId() { return  ID_MODE_POINTS; }

// ----------------------------------------------------------------------------- : Mouse events

void SymbolPointEditor::onLeftDown(const Vector2D& pos, wxMouseEvent& ev) {
	SelectedHandle handle = findHandle(pos);
	if (handle.handle) {
		selectHandle(handle, ev);
	} else if (hovering == SELECTED_LINE) {
		selectLine(ev);
	} else if (hovering == SELECTED_NEW_POINT) {
		selectLine(ev);
	} else {
		selectNothing();
	}
	// update window
	control.Refresh(false);
}

void SymbolPointEditor::onLeftUp(const Vector2D& pos, wxMouseEvent& ev) {
	// Left up => finalize all actions, new events start new actions
	resetActions();
}

void SymbolPointEditor::onLeftDClick(const Vector2D& pos, wxMouseEvent& ev) {
	findHoveredItem(pos, false);
	if (hovering == SELECTED_NEW_POINT) {
		// Add point
		ControlPointAddAction* act = new ControlPointAddAction(part, hoverLine1Idx, hoverLineT);
		getSymbol()->actions.add(act);
		// select the new point
		selectPoint(act->getNewPoint(), false);
		selection = SELECTED_POINTS;
	} else if (hovering == SELECTED_HANDLE && hoverHandle.handle == HANDLE_MAIN) { //%%%%%%% ||/&&
		// Delete point
		selectedPoints.clear();
		selectPoint(hoverHandle.point, false);
		getSymbol()->actions.add(controlPointRemoveAction(part, selectedPoints));
		selectedPoints.clear();
		selection = SELECTED_NONE;
	}
	// refresh
	findHoveredItem(pos, false);
	control.Refresh(false);
}

void SymbolPointEditor::onMouseMove(const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	// Moving the mouse without dragging => select a point/handle
	findHoveredItem(to, ev.AltDown());
	control.Refresh(false);
}

void SymbolPointEditor::onMouseDrag(const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	Vector2D delta = to - from;
	if (selection == SELECTED_LINE && ev.AltDown()) {
		// Drag the curve
		if (controlPointMoveAction) controlPointMoveAction = 0;
		if (!curveDragAction) {
			curveDragAction = new CurveDragAction(selectedLine1, selectedLine2);
			getSymbol()->actions.add(curveDragAction);
		}
		curveDragAction->move(delta, selectedLineT);
		control.Refresh(false);
	} else if (selection == SELECTED_POINTS || selection == SELECTED_LINE) {
		// Move all selected points
		if (curveDragAction)  curveDragAction = 0;
		if (!controlPointMoveAction) {
			// create action we can add this movement to
			controlPointMoveAction = new ControlPointMoveAction(selectedPoints);
			getSymbol()->actions.add(controlPointMoveAction);
		}
		controlPointMoveAction->constrain = ev.ControlDown(); // ctrl constrains
		controlPointMoveAction->move(delta);
		newPoint += delta;
		control.Refresh(false);
	} else if (selection == SELECTED_HANDLE) {
		// Move the selected handle
		if (!handleMoveAction) {
			handleMoveAction = new HandleMoveAction(selectedHandle);
			getSymbol()->actions.add(handleMoveAction);
		}
		handleMoveAction->constrain = ev.ControlDown(); // ctrl constrains
		handleMoveAction->move(delta);
		control.Refresh(false);
	}
}

// ----------------------------------------------------------------------------- : Other events


void SymbolPointEditor::onKeyChange(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_ALT && (hovering == SELECTED_LINE || hovering == SELECTED_NEW_POINT)) {
		if (ev.AltDown()) {
			hovering = SELECTED_LINE;
			control.SetCursor(pointCurve);
			SetStatusText(_("Drag to move curve"));
		} else {
			hovering = SELECTED_NEW_POINT;
			control.SetCursor(pointAdd);
			SetStatusText(_("Alt + drag to move curve;  double click to add control point on this line"));
		}
		control.Refresh(false);
	} else if (ev.GetKeyCode() == WXK_CONTROL) {
		// constrain changed
		if (controlPointMoveAction) {
			controlPointMoveAction->constrain = ev.ControlDown();
			controlPointMoveAction->move(Vector2D()); //refresh action
			control.Refresh(false);
		} else if (handleMoveAction) {
			handleMoveAction->constrain = ev.ControlDown();
			handleMoveAction->move(Vector2D()); //refresh action
			control.Refresh(false);
		}
	}
}

void SymbolPointEditor::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_DELETE) {
		deleteSelection();
	} else {
		ev.Skip();
	}
}

bool SymbolPointEditor::isEditing() {
	return handleMoveAction || controlPointMoveAction || curveDragAction;
}

// ----------------------------------------------------------------------------- : Selection

void SymbolPointEditor::selectNothing() {
	selection = SELECTED_NONE;
	selectedPoints.clear();
}

void SymbolPointEditor::selectPoint(const ControlPointP& point, bool toggle) {
	set<ControlPointP>::iterator inSet = selectedPoints.find(point);
	if (toggle) {
		if (inSet == selectedPoints.end()) {
			selectedPoints.insert(point);
		} else {
			selectedPoints.erase(inSet);
		}
	} else {
		if (inSet == selectedPoints.end()) {
			selectedPoints.clear();
			selectedPoints.insert(point);
		}
	}
}

void SymbolPointEditor::selectHandle(const SelectedHandle& h, const wxMouseEvent& keystate) {
	if (h.handle == HANDLE_MAIN) {
		selection = SELECTED_POINTS;
		selectPoint(h.point, keystate.ShiftDown());
	} else {
		selection = SELECTED_HANDLE;
		selectedHandle = h;
	}
}

void SymbolPointEditor::selectLine(const wxMouseEvent& keystate) {
	selection = SELECTED_LINE;
	selectedLine1 = hoverLine1;
	selectedLine2 = hoverLine2;
	selectedLineT = hoverLineT;
	if (!keystate.ShiftDown()) selectedPoints.clear();
	selectPoint(selectedLine1, true);
	selectPoint(selectedLine2, true);
}


bool SymbolPointEditor::pointSelected(const ControlPointP& pnt) {
	return selectedPoints.find(pnt) != selectedPoints.end();
}
bool SymbolPointEditor::pointSelected(const ControlPoint& pnt) {
	FOR_EACH(s, selectedPoints) {
		if (s.get() == &pnt) return true;
	}
	return false;
}

bool SymbolPointEditor::pointHovered(const ControlPointP& pnt) {
	return handleHovered(pnt, HANDLE_MAIN);
}
bool SymbolPointEditor::pointHovered(const ControlPoint&  pnt) {
	return handleHovered(pnt, HANDLE_MAIN);
}

bool SymbolPointEditor::handleHovered(const ControlPointP& pnt, WhichHandle wh) {
	return hovering == SELECTED_HANDLE && hoverHandle.point == pnt && hoverHandle.handle == wh;
}
bool SymbolPointEditor::handleHovered(const ControlPoint&  pnt, WhichHandle wh) {
	return hovering == SELECTED_HANDLE && hoverHandle.point && hoverHandle.point.get() == &pnt && hoverHandle.handle == wh;
}


// ----------------------------------------------------------------------------- : Actions

void SymbolPointEditor::resetActions() {
	handleMoveAction       = nullptr;
	controlPointMoveAction = nullptr;
	curveDragAction        = nullptr;
}

void SymbolPointEditor::deleteSelection() {
	if (!selectedPoints.empty()) {
		getSymbol()->actions.add(controlPointRemoveAction(part, selectedPoints));
		selectedPoints.clear();
		resetActions();
		control.Refresh(false);
	}
}

void SymbolPointEditor::onChangeSegment(SegmentMode mode) {
	assert(selectedLine1);
	assert(selectedLine2);
	if (selectedLine1->segmentAfter == mode) return;
	getSymbol()->actions.add(new SegmentModeAction(selectedLine1, selectedLine2, mode));
	control.Refresh(false);
}

void SymbolPointEditor::onChangeLock(LockMode mode) {
	getSymbol()->actions.add(new LockModeAction(*selectedPoints.begin(), mode));
	control.Refresh(false);
}


// ----------------------------------------------------------------------------- : Finding items

void SymbolPointEditor::findHoveredItem(const Vector2D& pos, bool altDown) {
	// is there a point currently under the cursor?
	hoverHandle = findHandle(pos);
	// change cursor and statusbar if point is under it
	if (hoverHandle.handle) {
		hovering = SELECTED_HANDLE;
		control.SetCursor(pointMove);
		SetStatusText(_("Click and drag to move control point"));
	} else {
		// Not on a point or handle, maybe the cursor is on a curve
		if (checkPosOnCurve(pos)) {
			if (altDown) {
				hovering = SELECTED_LINE;
				control.SetCursor(pointCurve);
				SetStatusText(_("Drag to move curve"));
			} else {
				hovering = SELECTED_NEW_POINT;
				control.SetCursor(pointAdd);
				SetStatusText(_("Alt + drag to move curve;  double click to add control point on this line"));
			}
		} else {
			hovering = SELECTED_NONE;
			control.SetCursor(*wxSTANDARD_CURSOR);
			SetStatusText(_(""));
		}
	}
}

bool SymbolPointEditor::checkPosOnCurve(const Vector2D& pos) {
	double range = control.rotation.trInvS(3); // less then 3 pixels away is still a hit
	size_t size = part->points.size();
	for(int i = 0 ; (size_t)i < size ; ++i) {
		// Curve between these lines
		hoverLine1 = part->getPoint(i);
		hoverLine2 = part->getPoint(i + 1);
		if (posOnSegment(pos, range, *hoverLine1, *hoverLine2, newPoint, hoverLineT)) {
			// mouse is on this line
			hoverLine1Idx = i;
			return true;
		}
	}
	return false;
}

SelectedHandle SymbolPointEditor::findHandle(const Vector2D& pos) {
	double range = control.rotation.trInvS(3); // less then 3 pixels away is still a hit
	// Is there a main handle there?
	FOR_EACH(p, part->points) {
		if (inRange(p->pos, pos, range)) {
			// point is at pos
			return SelectedHandle(p, HANDLE_MAIN);
		}
	}
	// Is there a sub handle there?
	// only check visible handles
	for (int i = 0 ; (size_t)i < part->points.size() ; ++i) {
		ControlPointP p = part->getPoint(i);
		bool sel    = pointSelected(p);
		bool before = sel || pointSelected(part->getPoint(i-1)); // are the handles visible?
		bool after  = sel || pointSelected(part->getPoint(i+1));
		if (before && p->segmentBefore == SEGMENT_CURVE) {
			if (inRange(p->pos + p->deltaBefore, pos, range)) {
				return SelectedHandle(p, HANDLE_BEFORE);
			}
		}
		if (after && p->segmentAfter == SEGMENT_CURVE) {
			if (inRange(p->pos + p->deltaAfter, pos, range)) {
				return SelectedHandle(p, HANDLE_AFTER);
			}
		}
	}
	// Nothing found
	return HANDLE_NONE;
}

bool SymbolPointEditor::inRange(const Vector2D& a, const Vector2D& b, double range) {
	return abs(a.x - b.x) <= range &&
	       abs(a.y - b.y) <= range;
}
