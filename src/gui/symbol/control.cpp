//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/control.hpp>
#include <gui/symbol/window.hpp>
#include <gui/symbol/editor.hpp>
#include <gui/symbol/select_editor.hpp>
#include <gui/symbol/point_editor.hpp>
#include <gui/symbol/basic_shape_editor.hpp>
#include <gui/util.hpp>
#include <data/action/symbol.hpp>
#include <util/window_id.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : SymbolControl

SymbolControl::SymbolControl(SymbolWindow* parent, int id, const SymbolP& symbol)
	: wxControl(parent, id)
	, SymbolViewer(symbol)
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
	selected_parts.clear();
	switchEditor(new_shared2<SymbolSelectEditor>(this, false));
	Refresh(false);
}

void SymbolControl::onModeChange(wxCommandEvent& ev) {
	switch (ev.GetId()) {
		case ID_MODE_SELECT:
			switchEditor(new_shared2<SymbolSelectEditor>(this, false));
			break;
		case ID_MODE_ROTATE:
			switchEditor(new_shared2<SymbolSelectEditor>(this, true));
			break;
		case ID_MODE_POINTS:
			if (selected_parts.size() == 1) {
				single_selection = *selected_parts.begin();
				switchEditor(new_shared2<SymbolPointEditor>(this, single_selection));
			}
			break;
		case ID_MODE_SHAPES:
			if (!selected_parts.empty()) {
				selected_parts.clear();
				signalSelectionChange();
			}
			switchEditor(new_shared1<SymbolBasicShapeEditor>(this));
			break;
	}
}

void SymbolControl::onExtraTool(wxCommandEvent& ev) {
	if (editor) editor->onCommand(ev.GetId());
}

void SymbolControl::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, SymbolPartAction) {
		Refresh(false);
	}
}

void SymbolControl::onUpdateSelection() {
	switch(editor->modeToolId()) {
		case ID_MODE_POINTS:
			// can only select a single part!
			if (selected_parts.size() > 1) {
				SymbolPartP part = *selected_parts.begin();
				selected_parts.clear();
				selected_parts.insert(part);
				signalSelectionChange();
			} else if (selected_parts.empty()) {
				selected_parts.insert(single_selection);
				signalSelectionChange();
			}
			if (single_selection != *selected_parts.begin()) {
				// begin editing another part
				single_selection = *selected_parts.begin();
				editor = new_shared2<SymbolPointEditor>(this, single_selection);
				Refresh(false);
			}
			break;
		case ID_MODE_SHAPES:
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
	selected_parts.clear();
	selected_parts.insert(part);
	switchEditor(new_shared2<SymbolSelectEditor>(this, false));
	signalSelectionChange();
}

void SymbolControl::activatePart(const SymbolPartP& part) {
	selected_parts.clear();
	selected_parts.insert(part);
	switchEditor(new_shared2<SymbolPointEditor>(this, part));
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
	// draw editing overlay
	if (editor) {
		editor->draw(dc);
	}
}
void SymbolControl::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
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
	wxSize s = ev.GetSize();
	rotation.setZoom(min(s.GetWidth(), s.GetHeight()));
	Refresh(false);
}
void SymbolControl::onUpdateUI(wxUpdateUIEvent& ev) {
	if (!editor) return;
	switch (ev.GetId()) {
		case ID_MODE_SELECT: case ID_MODE_ROTATE: case ID_MODE_POINTS: case ID_MODE_SHAPES: //case ID_MODE_PAINT:
			ev.Check(editor->modeToolId() == ev.GetId());
			if (ev.GetId() == ID_MODE_POINTS) {
				// can only edit points when a single part is selected <TODO?>
				ev.Enable(selected_parts.size() == 1);
			}
			break;
		case ID_MODE_PAINT:
			ev.Enable(false); // TODO
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