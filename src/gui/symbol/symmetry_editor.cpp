//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/symmetry_editor.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <data/settings.hpp>
#include <data/action/symbol.hpp>
#include <data/action/symbol_part.hpp>
#include <wx/spinctrl.h>

// ----------------------------------------------------------------------------- : SymbolSymmetryEditor

SymbolSymmetryEditor::SymbolSymmetryEditor(SymbolControl* control)
	: SymbolEditorBase(control)
	, mode(ID_SYMMETRY_ROTATION)
	, drawing(false)
{
	control->SetCursor(*wxCROSS_CURSOR);
}

// ----------------------------------------------------------------------------- : Drawing

void SymbolSymmetryEditor::draw(DC& dc) {
	if (symmetry) {
		control.highlightPart(dc, *symmetry);
	}
}

// ----------------------------------------------------------------------------- : UI

void SymbolSymmetryEditor::initUI(wxToolBar* tb, wxMenuBar* mb) {
	copies = new wxSpinCtrl(  tb, ID_COPIES, _("2"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 2, 10, 2);
	copies->SetHelpText(_HELP_("copies"));
	copies->SetSize(50, -1);
	tb->AddSeparator();
	tb->AddTool(ID_SYMMETRY_ROTATION,	_TOOL_("rotation"),		load_resource_image(_("symmetry_rotation")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("rotation"),   _HELP_("rotation"));
	tb->AddTool(ID_SYMMETRY_REFLECTION,	_TOOL_("reflection"),	load_resource_image(_("symmetry_reflection")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("reflection"), _HELP_("reflection"));
	tb->AddControl(copies);
	tb->Realize();
	control.SetCursor(*wxCROSS_CURSOR);
	stopActions(); // set status text
}

void SymbolSymmetryEditor::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	tb->DeleteTool(ID_SYMMETRY_REFLECTION);
	tb->DeleteTool(ID_SYMMETRY_ROTATION);
	// HACK: hardcoded size of rest of toolbar
	tb->DeleteToolByPos(7); // delete separator
	tb->DeleteTool(ID_COPIES); // delete sides
	#if wxVERSION_NUMBER < 2600
		delete sides;
	#endif
}

void SymbolSymmetryEditor::onUpdateUI(wxUpdateUIEvent& ev) {
	if (ev.GetId() >= ID_SYMMETRY && ev.GetId() < ID_SYMMETRY_MAX) {
		ev.Check(ev.GetId() == mode);
	} else if (ev.GetId() == ID_COPIES) {
		ev.Enable(true);
	} else {
		ev.Enable(false); // we don't know about this item
	}
}

void SymbolSymmetryEditor::onCommand(int id) {
	if (id >= ID_SYMMETRY && id < ID_SYMMETRY_MAX) {
		mode = id;
		stopActions();
	}
}

int SymbolSymmetryEditor::modeToolId() { return ID_MODE_SYMMETRY; }

// ----------------------------------------------------------------------------- : Mouse events

void SymbolSymmetryEditor::onLeftDown   (const Vector2D& pos, wxMouseEvent& ev) {
	// Start drawing
	drawing = true;
	center = handle = pos;
	SetStatusText(_HELP_("drag to draw symmetry"));
}

void SymbolSymmetryEditor::onLeftUp     (const Vector2D& pos, wxMouseEvent& ev) {
	if (drawing && symmetry) {
		// Finalize the symmetry
		getSymbol()->actions.add(new AddSymbolPartAction(*getSymbol(), symmetry));
		// Select the part
		control.selectPart(symmetry);
		// No need to clean up, this editor gets destroyed
		//stopActions();
	}
}

void SymbolSymmetryEditor::onMouseDrag  (const Vector2D& from, const Vector2D& to, wxMouseEvent& ev) {
	// Resize the object
	if (drawing) {
		handle = to;
		makePart(center, handle, ev.ControlDown(), settings.symbol_grid_snap != ev.ShiftDown());
		control.Refresh(false);
	}
}

// ----------------------------------------------------------------------------- : Other events

void SymbolSymmetryEditor::onKeyChange(wxKeyEvent& ev) {
	if (drawing) {
		if (ev.GetKeyCode() == WXK_CONTROL || ev.GetKeyCode() == WXK_SHIFT) {
			// changed constrains
			makePart(center, handle, ev.ControlDown(), settings.symbol_grid_snap != ev.ShiftDown());
			control.Refresh(false);
		} else if (ev.GetKeyCode() == WXK_ESCAPE) {
			// cancel drawing
			stopActions();
		}
	}
}

bool SymbolSymmetryEditor::isEditing() { return drawing; }

// ----------------------------------------------------------------------------- : Generating shapes

void SymbolSymmetryEditor::stopActions() {
	symmetry = SymbolSymmetryP();
	drawing = false;
	SetStatusText(_HELP_("draw symmetry"));
	control.Refresh(false);
}

void SymbolSymmetryEditor::makePart(Vector2D a, Vector2D b, bool constrained, bool snap) {
	// snap
	if (snap) {
		a = snap_vector(a, settings.symbol_grid_size);
		b = snap_vector(b, settings.symbol_grid_size);
	}
	// constrain
	Vector2D dir = b - a;
	if (constrained) {
		double angle = atan2(dir.y, dir.x);
		// multiples of 2pi/24 i.e. 24 stops
		double mult = (2 * M_PI) / 24;
		angle = floor(angle / mult + 0.5) * mult;
		dir = Vector2D(cos(angle), sin(angle)) * dir.length();
	}
	// make part
	if (!symmetry) {
		symmetry = new_intrusive<SymbolSymmetry>();
	}
	symmetry->kind   = mode == ID_SYMMETRY_ROTATION ? SYMMETRY_ROTATION : SYMMETRY_REFLECTION;
	symmetry->copies = copies->GetValue();
	symmetry->center = a;
	symmetry->handle = dir;
	symmetry->name   = capitalize(mode == ID_SYMMETRY_ROTATION ? _TYPE_("rotation") : _TYPE_("reflection"))
	                 + String::Format(_(" (%d)"), symmetry->copies);
}
