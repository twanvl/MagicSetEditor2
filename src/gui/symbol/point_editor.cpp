//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/point_editor.hpp>
#include <gui/symbol/window.hpp>
#include <gui/util.hpp>
#include <gfx/bezier.hpp>
#include <data/action/symbol_part.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(ControlPointP);

// ----------------------------------------------------------------------------- : SymbolPointEditor

SymbolPointEditor::SymbolPointEditor(SymbolControl* control, const SymbolShapeP& part)
	: SymbolEditorBase(control)
	, part(part)
	, selection(SELECTED_NONE)
	, hovering(SELECTED_NONE)
	// Load gui stock
	, pointSelect(load_resource_cursor(_("point")))
	, pointAdd   (load_resource_cursor(_("point_add")))
	, pointCurve (load_resource_cursor(_("curve")))
	, pointMove  (load_resource_cursor(_("point_move")))
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
	BezierCurve c(*hover_line_1, *hover_line_2);
	wxPoint prevPoint = control.rotation.tr(hover_line_1->pos);
	for(int i = 1 ; i <= 100 ; ++i) {
		// Draw 100 segments of the curve
		double t = double(i)/100.0f;
		wxPoint curPoint = control.rotation.tr(c.pointAt(t));
		double selectPercent = 1.0 - 1.2 * sqrt(fabs(hover_line_t-t)); // amount to color
		if (selectPercent > 0) {
			// gradient color
			Color color(
				col(300 - int(300 * selectPercent)),
				col(300 * int(selectPercent)),
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
	wxPoint p = control.rotation.tr(new_point);
	drawHandleBox(dc, p.x, p.y, true);
}

void SymbolPointEditor::drawControlPoint(DC& dc, const ControlPoint& p, bool drawHandleBefore, bool drawHandleAfter) {
	// Position
	wxPoint p0 = control.rotation.tr(p.pos);
	// Sub handles
	if (drawHandleBefore || drawHandleAfter) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		// Before handle
		if (drawHandleBefore && p.segment_before == SEGMENT_CURVE) {
			wxPoint p1 = control.rotation.tr(p.pos + p.delta_before);
			dc.SetPen(handlePen(PEN_LINE, p.lock));
			dc.DrawLine(p0.x, p0.y, p1.x, p1.y);
			dc.SetPen(handlePen(handleHovered(p, HANDLE_BEFORE) ? PEN_HOVER : PEN_NORMAL, p.lock));
			drawHandleCircle(dc, p1.x, p1.y);
		}
		// After handle
		if (drawHandleAfter && p.segment_after == SEGMENT_CURVE) {
			wxPoint p1 = control.rotation.tr(p.pos + p.delta_after);
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
	tb->AddTool(ID_SEGMENT_LINE,	_TOOL_("line segment"),		load_resource_tool_image(_("line")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("line segment"),		_HELP_("line segment"));
	tb->AddTool(ID_SEGMENT_CURVE,	_TOOL_("curve segment"),	load_resource_tool_image(_("curve")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("curve segment"),		_HELP_("curve segment"));
	tb->AddSeparator();																													
	tb->AddTool(ID_LOCK_FREE,		_TOOL_("free point"),		load_resource_tool_image(_("lock_free")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("free point"),		_HELP_("free point"));
	tb->AddTool(ID_LOCK_DIR,		_TOOL_("smooth point"),		load_resource_tool_image(_("lock_dir")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("smooth point"),		_HELP_("smooth point"));
	tb->AddTool(ID_LOCK_SIZE,		_TOOL_("symmetric point"),	load_resource_tool_image(_("lock_size")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("symmetric point"),	_HELP_("symmetric point"));
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
	tb->DeleteToolByPos(7); // delete separator
	tb->DeleteToolByPos(7); // delete separator
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
				        selected_points.size() == 1 &&
				        (*selected_points.begin())->segment_before == SEGMENT_CURVE &&
				        (*selected_points.begin())->segment_after  == SEGMENT_CURVE;
			break;
		default:
			ev.Enable(false); // we don't know this item
			return;
	}
	// check
	if (enabled) {
		switch (ev.GetId()) {
			case ID_SEGMENT_LINE: case ID_SEGMENT_CURVE:
				checked = selected_line1->segment_after == ev.GetId() - ID_SEGMENT;
				break;
			case ID_LOCK_FREE: case ID_LOCK_DIR: case ID_LOCK_SIZE:
				checked = (*selected_points.begin())->lock == ev.GetId() - ID_LOCK;
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
			break;
		case ID_LOCK_FREE: case ID_LOCK_DIR: case ID_LOCK_SIZE:
			onChangeLock( static_cast<LockMode>(id - ID_LOCK) );
			break;
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
		ControlPointAddAction* act = new ControlPointAddAction(part, hover_line_1_idx, hover_line_t);
		getSymbol()->actions.add(act);
		// select the new point
		selectPoint(act->getNewPoint(), false);
		selection = SELECTED_POINTS;
	} else if (hovering == SELECTED_HANDLE && hover_handle.handle == HANDLE_MAIN) {
		// Delete point
		selected_points.clear();
		selectPoint(hover_handle.point, false);
		getSymbol()->actions.add(control_point_remove_action(part, selected_points));
		selected_points.clear();
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

template <typename Event> int snap(Event& ev) {
	return settings.symbol_grid_snap != ev.ShiftDown() ? settings.symbol_grid_size : 0; // shift toggles snap
}

void SymbolPointEditor::onMouseDrag(const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	Vector2D delta = to - from;
	if (selection == SELECTED_LINE && ev.AltDown()) {
		// Drag the curve
		if (controlPointMoveAction) controlPointMoveAction = 0;
		if (!curveDragAction) {
			curveDragAction = new CurveDragAction(selected_line1, selected_line2);
			getSymbol()->actions.add(curveDragAction);
		}
		curveDragAction->move(delta, selected_line_t);
		control.Refresh(false);
	} else if (selection == SELECTED_POINTS || selection == SELECTED_LINE) {
		// Move all selected points
		if (curveDragAction)  curveDragAction = 0;
		if (!controlPointMoveAction) {
			// create action we can add this movement to
			controlPointMoveAction = new ControlPointMoveAction(selected_points);
			getSymbol()->actions.add(controlPointMoveAction);
		}
		controlPointMoveAction->constrain = ev.ControlDown(); // ctrl constrains
		controlPointMoveAction->snap      = snap(ev);
		controlPointMoveAction->move(delta);
		new_point += delta;
		control.Refresh(false);
	} else if (selection == SELECTED_HANDLE) {
		// Move the selected handle
		if (!handleMoveAction) {
			handleMoveAction = new HandleMoveAction(selected_handle);
			getSymbol()->actions.add(handleMoveAction);
		}
		handleMoveAction->constrain  = ev.ControlDown(); // ctrl constrains
		handleMoveAction->snap = snap(ev);
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
			SetStatusText(_HELP_("drag to move curve"));
		} else {
			hovering = SELECTED_NEW_POINT;
			control.SetCursor(pointAdd);
			SetStatusText(_HELP_("drag to move line"));
		}
		control.Refresh(false);
	} else if (ev.GetKeyCode() == WXK_CONTROL || ev.GetKeyCode() == WXK_SHIFT) {
		// constrain/snap changed
		if (controlPointMoveAction) {
			controlPointMoveAction->constrain = ev.ControlDown();
			controlPointMoveAction->snap = snap(ev);
			controlPointMoveAction->move(Vector2D()); //refresh action
			control.Refresh(false);
		} else if (handleMoveAction) {
			handleMoveAction->constrain = ev.ControlDown();
			handleMoveAction->snap = snap(ev);
			handleMoveAction->move(Vector2D()); //refresh action
			control.Refresh(false);
		}
	}
}

void SymbolPointEditor::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_DELETE) {
		deleteSelection();
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
		// what to move
		if (selection == SELECTED_POINTS || selection == SELECTED_LINE) {
			// Move all selected points
			controlPointMoveAction = new ControlPointMoveAction(selected_points);
			getSymbol()->actions.add(controlPointMoveAction);
			controlPointMoveAction->move(delta);
			new_point += delta;
			control.Refresh(false);
		} else if (selection == SELECTED_HANDLE) {
			// Move the selected handle
			handleMoveAction = new HandleMoveAction(selected_handle);
			getSymbol()->actions.add(handleMoveAction);
			handleMoveAction->move(delta);
			control.Refresh(false);
		}
		resetActions();
	}
}

bool SymbolPointEditor::isEditing() {
	return handleMoveAction || controlPointMoveAction || curveDragAction;
}

// ----------------------------------------------------------------------------- : Selection

void SymbolPointEditor::selectNothing() {
	selection = SELECTED_NONE;
	selected_points.clear();
}

void SymbolPointEditor::selectPoint(const ControlPointP& point, bool toggle) {
	set<ControlPointP>::iterator inSet = selected_points.find(point);
	if (toggle) {
		if (inSet == selected_points.end()) {
			selected_points.insert(point);
		} else {
			selected_points.erase(inSet);
		}
	} else {
		if (inSet == selected_points.end()) {
			selected_points.clear();
			selected_points.insert(point);
		}
	}
}

void SymbolPointEditor::selectHandle(const SelectedHandle& h, const wxMouseEvent& keystate) {
	if (h.handle == HANDLE_MAIN) {
		selection = SELECTED_POINTS;
		selectPoint(h.point, keystate.ShiftDown());
	} else {
		selection = SELECTED_HANDLE;
		selected_handle = h;
	}
}

void SymbolPointEditor::selectLine(const wxMouseEvent& keystate) {
	selection = SELECTED_LINE;
	selected_line1 = hover_line_1;
	selected_line2 = hover_line_2;
	selected_line_t = hover_line_t;
	if (!keystate.ShiftDown()) selected_points.clear();
	selectPoint(selected_line1, true);
	selectPoint(selected_line2, true);
}


bool SymbolPointEditor::pointSelected(const ControlPointP& pnt) {
	return selected_points.find(pnt) != selected_points.end();
}
bool SymbolPointEditor::pointSelected(const ControlPoint& pnt) {
	FOR_EACH(s, selected_points) {
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
	return hovering == SELECTED_HANDLE && hover_handle.point == pnt && hover_handle.handle == wh;
}
bool SymbolPointEditor::handleHovered(const ControlPoint&  pnt, WhichHandle wh) {
	return hovering == SELECTED_HANDLE && hover_handle.point && hover_handle.point.get() == &pnt && hover_handle.handle == wh;
}


// ----------------------------------------------------------------------------- : Actions

void SymbolPointEditor::resetActions() {
	handleMoveAction       = nullptr;
	controlPointMoveAction = nullptr;
	curveDragAction        = nullptr;
}

void SymbolPointEditor::deleteSelection() {
	if (!selected_points.empty()) {
		getSymbol()->actions.add(control_point_remove_action(part, selected_points));
		selected_points.clear();
		resetActions();
		control.Refresh(false);
	}
}

void SymbolPointEditor::onChangeSegment(SegmentMode mode) {
	assert(selected_line1);
	assert(selected_line2);
	if (selected_line1->segment_after == mode) return;
	getSymbol()->actions.add(new SegmentModeAction(selected_line1, selected_line2, mode));
	control.Refresh(false);
}

void SymbolPointEditor::onChangeLock(LockMode mode) {
	getSymbol()->actions.add(new LockModeAction(*selected_points.begin(), mode));
	control.Refresh(false);
}


// ----------------------------------------------------------------------------- : Finding items

void SymbolPointEditor::findHoveredItem(const Vector2D& pos, bool altDown) {
	// is there a point currently under the cursor?
	hover_handle = findHandle(pos);
	// change cursor and statusbar if point is under it
	if (hover_handle.handle) {
		hovering = SELECTED_HANDLE;
		control.SetCursor(pointMove);
		SetStatusText(_HELP_("drag to move point"));
	} else {
		// Not on a point or handle, maybe the cursor is on a curve
		if (checkPosOnCurve(pos)) {
			if (altDown) {
				hovering = SELECTED_LINE;
				control.SetCursor(pointCurve);
				SetStatusText(_HELP_("drag to move curve"));
			} else {
				hovering = SELECTED_NEW_POINT;
				control.SetCursor(pointAdd);
				SetStatusText(_HELP_("drag to move line"));
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
		hover_line_1 = part->getPoint(i);
		hover_line_2 = part->getPoint(i + 1);
		if (pos_on_segment(pos, range, *hover_line_1, *hover_line_2, new_point, hover_line_t)) {
			// mouse is on this line
			hover_line_1_idx = i;
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
		if (before && p->segment_before == SEGMENT_CURVE) {
			if (inRange(p->pos + p->delta_before, pos, range)) {
				return SelectedHandle(p, HANDLE_BEFORE);
			}
		}
		if (after && p->segment_after == SEGMENT_CURVE) {
			if (inRange(p->pos + p->delta_after, pos, range)) {
				return SelectedHandle(p, HANDLE_AFTER);
			}
		}
	}
	// Nothing found
	return HANDLE_NONE;
}

bool SymbolPointEditor::inRange(const Vector2D& a, const Vector2D& b, double range) {
	return fabs(a.x - b.x) <= range &&
	       fabs(a.y - b.y) <= range;
}
