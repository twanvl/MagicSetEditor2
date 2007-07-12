//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/control.hpp>
#include <gui/symbol/window.hpp>
#include <gui/symbol/editor.hpp>
#include <gui/symbol/select_editor.hpp>
#include <gui/symbol/point_editor.hpp>
#include <gui/symbol/basic_shape_editor.hpp>
#include <gui/symbol/symmetry_editor.hpp>
#include <gui/util.hpp>
#include <data/action/symbol.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : SymbolControl

SymbolControl::SymbolControl(SymbolWindow* parent, int id, const SymbolP& symbol)
	: wxControl(parent, id)
	, SymbolViewer(symbol, true)
	, parent(parent)
{
	onChangeSymbol();
}

void SymbolControl::switchEditor(const SymbolEditorBaseP& e) {
	if (editor) editor->destroyUI(parent->GetToolBar(), parent->GetMenuBar());
	editor = e;
	if (editor) editor->initUI   (parent->GetToolBar(), parent->GetMenuBar());
	Refresh(false);
}

void SymbolControl::onChangeSymbol() {
	selected_parts.setSymbol(symbol);
	switchEditor(new_intrusive2<SymbolSelectEditor>(this, false));
	Refresh(false);
}

void SymbolControl::onModeChange(wxCommandEvent& ev) {
	switch (ev.GetId()) {
		case ID_MODE_SELECT:
			switchEditor(new_intrusive2<SymbolSelectEditor>(this, false));
			break;
		case ID_MODE_ROTATE:
			switchEditor(new_intrusive2<SymbolSelectEditor>(this, true));
			break;
		case ID_MODE_POINTS:
			if (selected_parts.size() == 1) {
				selected_shape = selected_parts.getAShape();
				if (selected_shape) {
					switchEditor(new_intrusive2<SymbolPointEditor>(this, selected_shape));
				}
			}
			break;
		case ID_MODE_SHAPES:
			if (!selected_parts.empty()) {
				selected_parts.clear();
				signalSelectionChange();
			}
			switchEditor(new_intrusive1<SymbolBasicShapeEditor>(this));
			break;
		case ID_MODE_SYMMETRY:
			switchEditor(new_intrusive2<SymbolSymmetryEditor>(this, selected_parts.getASymmetry()));
			break;
	}
}

void SymbolControl::onExtraTool(wxCommandEvent& ev) {
	switch (ev.GetId()) {
		case ID_VIEW_GRID:
			settings.symbol_grid = !settings.symbol_grid;
			Refresh(false);
			break;
		case ID_VIEW_GRID_SNAP:
			settings.symbol_grid_snap = !settings.symbol_grid_snap;
			Refresh(false);
			break;
		default:
			if (editor) editor->onCommand(ev.GetId());
	}
}

void SymbolControl::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, SymbolPartAction) {
		Refresh(false);
	}
}

void SymbolControl::onUpdateSelection() {
	switch(editor->modeToolId()) {
		case ID_MODE_POINTS: {
			// can only select a single part!
			SymbolShapeP shape = selected_parts.getAShape();
			if (!shape) {
				if (selected_parts.select(selected_shape)) {
					signalSelectionChange();
				}
				break;
			}
			if (shape != selected_shape) {
				if (selected_parts.select(shape)) {
					signalSelectionChange();
				}
				// begin editing another part
				selected_shape = shape;
				editor = new_intrusive2<SymbolPointEditor>(this, selected_shape);
				Refresh(false);
			}
			break;
		} case ID_MODE_SYMMETRY: {
			// can only select a single part!
			SymbolSymmetryP symmetry = selected_parts.getASymmetry();
			if (!symmetry) {
				if (selected_symmetry && selected_parts.select(selected_symmetry)) {
					signalSelectionChange();
				}
				break;
			}
			if (symmetry != selected_symmetry) {
				if (symmetry && selected_parts.select(symmetry)) {
					signalSelectionChange();
				}
				// begin editing another part
				selected_symmetry = symmetry;
				Refresh(false);
			}
			break;
		} case ID_MODE_SHAPES:
			if (!selected_parts.empty()) {
				// there can't be a selection
				selected_parts.clear();
				signalSelectionChange();
			}
			break;
		default:
			Refresh(false);
			break;
	}
}

void SymbolControl::selectPart(const SymbolPartP& part) {
	selected_parts.select(part);
	switchEditor(new_intrusive2<SymbolSelectEditor>(this, false));
	signalSelectionChange();
}

void SymbolControl::activatePart(const SymbolPartP& part) {
	if (part->isSymbolShape()) {
		selected_parts.select(part);
		switchEditor(new_intrusive2<SymbolPointEditor>(this, static_pointer_cast<SymbolShape>(part)));
	} else if (part->isSymbolSymmetry()) {
		selected_parts.select(part);
		switchEditor(new_intrusive2<SymbolSymmetryEditor>(this, static_pointer_cast<SymbolSymmetry>(part)));
	}
}

void SymbolControl::signalSelectionChange() {
	parent->onSelectFromControl();
}

bool SymbolControl::isEditing() {
	return editor && editor->isEditing();
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolControl::draw(DC& dc) {
	// clear the background
	clearDC(dc, Color(0, 128, 0));
	// draw symbol iself
	SymbolViewer::draw(dc);
	// draw grid
	if (settings.symbol_grid) {
		RotatedDC rdc(dc, rotation, QUALITY_LOW);
		double lines = settings.symbol_grid_size;
		for (int i = 0 ; i <= lines ; ++i) {
			double x = (double)i/lines;
			rdc.SetLogicalFunction(wxAND);
			rdc.SetPen(i%5 == 0 ? Color(191,255,191) : Color(191, 255, 191));
			rdc.DrawLine(RealPoint(0,x), RealPoint(1,x));
			rdc.DrawLine(RealPoint(x,0), RealPoint(x,1));
			rdc.SetLogicalFunction(wxOR);
			rdc.SetPen(i%5 == 0 ? Color(0,63,0) : Color(0, 31, 0));
			rdc.DrawLine(RealPoint(0,x), RealPoint(1,x));
			rdc.DrawLine(RealPoint(x,0), RealPoint(x,1));
		}
		dc.SetLogicalFunction(wxCOPY);
	}
	// draw editing overlay
	if (editor) {
		editor->draw(dc);
	}
}
void SymbolControl::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	draw(dc);
}

// ----------------------------------------------------------------------------- : Events

// Mouse events, convert position, forward event

void SymbolControl::onLeftDown(wxMouseEvent& ev) {
	Vector2D pos = rotation.trInv(RealPoint(ev.GetX(), ev.GetY()));
	if (editor) editor->onLeftDown(pos, ev);
	last_pos = pos;
	ev.Skip(); // for focus
}
void SymbolControl::onLeftUp(wxMouseEvent& ev) {
	Vector2D pos = rotation.trInv(RealPoint(ev.GetX(), ev.GetY()));
	if (editor) editor->onLeftUp(pos, ev);
	last_pos = pos;
}
void SymbolControl::onLeftDClick(wxMouseEvent& ev) {
	Vector2D pos = rotation.trInv(RealPoint(ev.GetX(), ev.GetY()));
	if (editor) editor->onLeftDClick(pos, ev);
	last_pos = pos;
}
void SymbolControl::onRightDown(wxMouseEvent& ev) {
	Vector2D pos = rotation.trInv(RealPoint(ev.GetX(), ev.GetY()));
	if (editor) editor->onRightDown(pos, ev);
	last_pos = pos;
}

void SymbolControl::onMotion(wxMouseEvent& ev) {
	Vector2D pos = rotation.trInv(RealPoint(ev.GetX(), ev.GetY()));
	// Dragging something?
	if (ev.LeftIsDown()) {
		if (editor) editor->onMouseDrag(last_pos, pos, ev);
	} else {
		if (editor) editor->onMouseMove(last_pos, pos, ev);
	}
	last_pos = pos;
}

// Key events, just forward

void SymbolControl::onKeyChange(wxKeyEvent& ev) {
	if (editor) editor->onKeyChange(ev);
	ev.Skip(); // so we get char events
}
void SymbolControl::onChar(wxKeyEvent& ev) {
	if (editor) editor->onChar(ev);
	else        ev.Skip();
}

void SymbolControl::onSize(wxSizeEvent& ev) {
	wxSize s = GetClientSize();
	int zoom = min(s.x, s.y);
	setZoom(zoom);
	if (s.x > zoom) {
		setOrigin(Vector2D((s.x - zoom) * 0.5, 0));
	} else {
		setOrigin(Vector2D(0, (s.y - zoom) * 0.5));
	}
	Refresh(false);
}
void SymbolControl::onUpdateUI(wxUpdateUIEvent& ev) {
	if (!editor) return;
	switch (ev.GetId()) {
		case ID_MODE_SELECT: case ID_MODE_ROTATE: case ID_MODE_POINTS:
		case ID_MODE_SHAPES: case ID_MODE_SYMMETRY: //case ID_MODE_PAINT:
			ev.Check(editor->modeToolId() == ev.GetId());
			if (ev.GetId() == ID_MODE_POINTS) {
				// can only edit points when a shape is available
				ev.Enable(selected_parts.getAShape());
			}
			if (ev.GetId() == ID_MODE_SYMMETRY) {
				ev.Enable(!selected_parts.empty());
			}
			break;
		case ID_MODE_PAINT:
			ev.Enable(false); // TODO
			break;
		case ID_VIEW_GRID:
			ev.Check(settings.symbol_grid);
			break;
		case ID_VIEW_GRID_SNAP:
			ev.Check(settings.symbol_grid_snap);
			break;
		default:
			if (ev.GetId() >= ID_CHILD_MIN && ev.GetId() < ID_CHILD_MAX) {
				editor->onUpdateUI(ev); // foward to editor
			}
	}
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(SymbolControl, wxControl)
	EVT_PAINT          (SymbolControl::onPaint)
	EVT_SIZE           (SymbolControl::onSize)
	EVT_LEFT_UP        (SymbolControl::onLeftUp)
	EVT_LEFT_DOWN      (SymbolControl::onLeftDown)
	EVT_RIGHT_DOWN     (SymbolControl::onRightDown)
	EVT_LEFT_DCLICK    (SymbolControl::onLeftDClick)
	EVT_MOTION         (SymbolControl::onMotion)
	EVT_KEY_UP         (SymbolControl::onKeyChange)
	EVT_KEY_DOWN       (SymbolControl::onKeyChange)
	EVT_CHAR           (SymbolControl::onChar)
END_EVENT_TABLE  ()
