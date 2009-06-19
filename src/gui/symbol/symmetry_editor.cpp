//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/symmetry_editor.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <data/settings.hpp>
#include <data/action/symbol.hpp>
#include <data/action/symbol_part.hpp>
#include <wx/spinctrl.h>

// ----------------------------------------------------------------------------- : SymbolSymmetryEditor

SymbolSymmetryEditor::SymbolSymmetryEditor(SymbolControl* control, const SymbolSymmetryP& sym)
	: SymbolEditorBase(control)
	, symmetry(control->selected_symmetry)
	, symmetryMoveAction(nullptr)
{
	symmetry = sym;
	control->selected_symmetry = symmetry;
	control->SetCursor(*wxCROSS_CURSOR);
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolSymmetryEditor::draw(DC& dc) {
	if (symmetry) {
		control.highlightPart(dc, *symmetry, HIGHLIGHT_BORDER);
		Color color(255,100,0);
		Vector2D center = control.rotation.tr(symmetry->center);
		Vector2D handle = control.rotation.tr(symmetry->center + symmetry->handle);
		if (symmetry->kind == SYMMETRY_REFLECTION) {
			// draw line to handle
			dc.SetPen(wxPen(color,1,wxDOT));
			dc.DrawLine(int(center.x), int(center.y), int(handle.x), int(handle.y));
			// draw handle
			dc.SetPen(*wxBLACK_PEN);
			dc.SetBrush(color);
			dc.DrawCircle(int(handle.x), int(handle.y), hovered == SELECTION_HANDLE ? 7 : 4);
		}
		// draw center
		dc.SetPen(*wxBLACK_PEN);
		dc.SetBrush(color);
		dc.DrawCircle(int(center.x), int(center.y), hovered == SELECTION_CENTER ? 8 : 5);
	}
}

// ----------------------------------------------------------------------------- : UI

void SymbolSymmetryEditor::initUI(wxToolBar* tb, wxMenuBar* mb) {
	copies = new wxSpinCtrl(  tb, ID_COPIES, _("2"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 2, 10, 2);
	copies->SetHelpText(_HELP_("copies"));
	copies->SetSize(50, -1);
	tb->AddSeparator();
	tb->AddTool(ID_ADD_SYMMETRY,		_TOOL_("add symmetry"),		load_resource_tool_image(_("symmetry_add")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("add symmetry"),    _HELP_("add symmetry"));
	tb->AddTool(ID_REMOVE_SYMMETRY,		_TOOL_("remove symmetry"),	load_resource_tool_image(_("symmetry_remove")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("remove symmetry"), _HELP_("remove symmetry"));
	tb->AddSeparator();
	tb->AddTool(ID_SYMMETRY_ROTATION,	_TOOL_("rotation"),		load_resource_image(_("symmetry_rotation")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("rotation"),   _HELP_("rotation"));
	tb->AddTool(ID_SYMMETRY_REFLECTION,	_TOOL_("reflection"),	load_resource_image(_("symmetry_reflection")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("reflection"), _HELP_("reflection"));
	tb->AddSeparator();
	tb->AddControl(copies);
	tb->Realize();
	control.SetCursor(*wxCROSS_CURSOR);
	resetActions(); // set status text
}

void SymbolSymmetryEditor::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->DeleteTool(ID_SYMMETRY_REFLECTION);
	tb->DeleteTool(ID_SYMMETRY_ROTATION);
	tb->DeleteTool(ID_ADD_SYMMETRY);
	tb->DeleteTool(ID_REMOVE_SYMMETRY);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(7); // delete separator
	tb->DeleteToolByPos(7); // delete separator
	tb->DeleteToolByPos(7); // delete separator
	tb->DeleteTool(ID_COPIES); // delete copies
	#if wxVERSION_NUMBER < 2600
		delete copies;
	#endif
}

void SymbolSymmetryEditor::onUpdateUI(wxUpdateUIEvent& ev) {
	if (ev.GetId() >= ID_SYMMETRY && ev.GetId() < ID_SYMMETRY_MAX) {
		ev.Enable(symmetry);
		ev.Check(symmetry && symmetry->kind == ev.GetId() - ID_SYMMETRY);
	} else if (ev.GetId() == ID_COPIES) {
		ev.Enable(symmetry);
		if (symmetry) {
			copies->SetValue(symmetry->copies);
		}
	} else if (ev.GetId() == ID_ADD_SYMMETRY) {
		ev.Enable(true);
	} else if (ev.GetId() == ID_REMOVE_SYMMETRY) {
		ev.Enable(symmetry);
	} else {
		ev.Enable(false); // we don't know about this item
	}
}

void SymbolSymmetryEditor::onCommand(int id) {
	if (id >= ID_SYMMETRY && id < ID_SYMMETRY_MAX) {
		SymbolSymmetryType kind = id == ID_SYMMETRY_ROTATION ? SYMMETRY_ROTATION : SYMMETRY_REFLECTION;
		if (symmetry && symmetry->kind != kind) {
			addAction(new SymmetryTypeAction(*symmetry, kind));
			control.Refresh(false);
		}
		resetActions();
	} else if (id == ID_COPIES) {
		if (symmetry && symmetry->copies != copies->GetValue()) {
			addAction(new SymmetryCopiesAction(*symmetry, copies->GetValue()));
			control.Refresh(false);
		}
		resetActions();
	} else if (id == ID_ADD_SYMMETRY) {
		symmetry = new_intrusive<SymbolSymmetry>();
		symmetry->kind   = SYMMETRY_ROTATION;
		symmetry->copies = 2;
		symmetry->center = Vector2D(0.5,0.5);
		symmetry->handle = Vector2D(0.2,0);
		symmetry->name   = symmetry->expectedName();
		addAction(new GroupSymbolPartsAction(*getSymbol(), control.selected_parts.get(), symmetry));
		control.selected_parts.select(symmetry);
		control.Refresh(false);
	} else if (id == ID_REMOVE_SYMMETRY) {
		addAction(new UngroupSymbolPartsAction(*getSymbol(), control.selected_parts.get()));
		symmetry = SymbolSymmetryP();
		control.Refresh(false);
	}
}

int SymbolSymmetryEditor::modeToolId() { return ID_MODE_SYMMETRY; }

// ----------------------------------------------------------------------------- : Mouse events

void SymbolSymmetryEditor::onLeftDown   (const Vector2D& pos, wxMouseEvent& ev) {
	if (!symmetry) return;
	selection = findSelection(pos);
}

void SymbolSymmetryEditor::onLeftUp     (const Vector2D& pos, wxMouseEvent& ev) {
	if (!symmetry) return;
	if (isEditing()) {
		resetActions();
	}
}

void SymbolSymmetryEditor::onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	if (!symmetry) return;
	// Resize the object
	if (selection == SELECTION_NONE) return;
	if (!symmetryMoveAction) {
		symmetryMoveAction = new SymmetryMoveAction(*symmetry, selection == SELECTION_HANDLE);
		symmetryMoveAction->constrain = ev.ControlDown();
		symmetryMoveAction->snap      = ev.ShiftDown() != settings.symbol_grid_snap ? settings.symbol_grid_size : 0;
		addAction(symmetryMoveAction);
	}
	symmetryMoveAction->move(to - from);
	control.Refresh(false);
}

void SymbolSymmetryEditor::onMouseMove  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	Selection old_hovered = hovered;
	hovered = findSelection(to);
	if (hovered != old_hovered) control.Refresh(false);
	// TODO: set status text
}

SymbolSymmetryEditor::Selection SymbolSymmetryEditor::findSelection(const Vector2D& pos) {
	if (!symmetry) return SELECTION_NONE;
	Vector2D pos_pixel = control.rotation.tr(pos);
	Vector2D center = control.rotation.tr(symmetry->center);
	if ((center - pos_pixel).lengthSqr() < 5*5) return SELECTION_CENTER;
	if (symmetry->kind == SYMMETRY_REFLECTION) {
		Vector2D handle = control.rotation.tr(symmetry->center + symmetry->handle);
		if ((handle - pos_pixel).lengthSqr() < 5*5) return SELECTION_HANDLE;
	}
	return SELECTION_NONE;
}

// ----------------------------------------------------------------------------- : Other events

void SymbolSymmetryEditor::onKeyChange(wxKeyEvent& ev) {
	if (symmetryMoveAction) {
		if (ev.GetKeyCode() == WXK_CONTROL || ev.GetKeyCode() == WXK_SHIFT) {
			// changed constrains
			symmetryMoveAction->constrain = ev.ControlDown();
			symmetryMoveAction->snap      = ev.ShiftDown() != settings.symbol_grid_snap ? settings.symbol_grid_size : 0;
			control.Refresh(false);
		} else if (ev.GetKeyCode() == WXK_ESCAPE) {
			// cancel drawing
			resetActions();
		}
	}
}

bool SymbolSymmetryEditor::isEditing() {
	return symmetryMoveAction;
}

void SymbolSymmetryEditor::resetActions() {
	symmetryMoveAction = nullptr;
}
