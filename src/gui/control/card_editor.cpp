//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_editor.hpp>
#include <gui/value/editor.hpp>
#include <gui/icon_menu.hpp>
#include <data/field.hpp>
#include <data/stylesheet.hpp>
#include <data/settings.hpp>

DECLARE_TYPEOF_COLLECTION(ValueViewerP);

// ----------------------------------------------------------------------------- : DataEditor

#define FOR_EACH_EDITOR							\
	FOR_EACH(v, viewers)						\
		if (ValueEditor* e = v->getEditor())
#define FOR_EACH_EDITOR_REVERSE					\
	FOR_EACH_REVERSE(v, viewers)				\
		if (ValueEditor* e = v->getEditor())

DataEditor::DataEditor(Window* parent, int id, long style)
	: CardViewer(parent, id, style)
	, current_viewer(nullptr)
	, current_editor(nullptr)
{}

ValueViewerP DataEditor::makeViewer(const StyleP& style) {
	return style->makeEditor(*this, style);
}

// ----------------------------------------------------------------------------- : Utility for ValueViewers

bool DataEditor::drawBorders() const {
	return !nativeLook() && settings.stylesheetSettingsFor(*set->stylesheetFor(card)).card_borders();
}
bool DataEditor::drawEditing() const {
	return true;
}

wxPen DataEditor::borderPen(bool active) const {
	return active ? wxPen(Color(0,128,255),   1, wxSOLID)
	              : wxPen(Color(128,128,128), 1, wxDOT);
}

ValueViewer* DataEditor::focusedViewer() const {
	return current_viewer;
}

// ----------------------------------------------------------------------------- : Selection

// ----------------------------------------------------------------------------- : Clipboard & Formatting

bool DataEditor::canCut()            const { return current_editor && current_editor->canCut();        }
bool DataEditor::canCopy()           const { return current_editor && current_editor->canCopy();       }
bool DataEditor::canPaste()          const { return current_editor && current_editor->canPaste();      }
bool DataEditor::canFormat(int type) const { return current_editor && current_editor->canFormat(type); }
bool DataEditor::hasFormat(int type) const { return current_editor && current_editor->hasFormat(type); }

void DataEditor::doCut()                   { if    (current_editor)   current_editor->doCut();         }
void DataEditor::doCopy()                  { if    (current_editor)   current_editor->doCopy();        }
void DataEditor::doPaste()                 { if    (current_editor)   current_editor->doPaste();       }
void DataEditor::doFormat(int type)        { if    (current_editor)   current_editor->doFormat(type);  }

// ----------------------------------------------------------------------------- : Mouse events

void DataEditor::onLeftDown(wxMouseEvent& ev) {
	ev.Skip(); // for focus
	CaptureMouse();
	// change selection?
	selectField(ev, &ValueEditor::onLeftDown);
}
void DataEditor::onLeftUp(wxMouseEvent& ev) {
	if (HasCapture()) ReleaseMouse();
	if (current_editor) current_editor->onLeftUp(mousePoint(ev), ev);
}
void DataEditor::onLeftDClick(wxMouseEvent& ev) {
	if (current_editor) current_editor->onLeftDClick(mousePoint(ev), ev);
}
void DataEditor::onRightDown(wxMouseEvent& ev) {
	ev.Skip(); // for context menu
	// change selection?
	selectField(ev, &ValueEditor::onRightDown);
}
void DataEditor::onMouseWheel(wxMouseEvent& ev) {
	if (current_editor) current_editor->onMouseWheel(mousePoint(ev), ev);
}

void DataEditor::onMotion(wxMouseEvent& ev) {
	RealPoint pos = mousePoint(ev);
	if (current_editor && ev.LeftIsDown()) {
		current_editor->onMotion(pos, ev);
	}
	if (!HasCapture()) {
		// change cursor and set status text
		wxFrame* frame = dynamic_cast<wxFrame*>( wxGetTopLevelParent(this) );
		FOR_EACH_EDITOR_REVERSE { // find high z index fields first
			if (v->containsPoint(pos) && v->getField()->editable) {
				wxCursor c = e->cursor();
				if (c.Ok()) SetCursor(c);
				else        SetCursor(wxCURSOR_ARROW);
				if (frame) frame->SetStatusText(v->getField()->description);
				return;
			}
		}
		// no field under cursor
		SetCursor(wxCURSOR_ARROW);
		if (frame) frame->SetStatusText(wxEmptyString);
	}
}

void DataEditor::onMouseLeave(wxMouseEvent& ev) {
	wxFrame* frame = dynamic_cast<wxFrame*>( wxGetTopLevelParent(this) );
	if (frame) frame->SetStatusText(wxEmptyString);
}

void DataEditor::selectField(wxMouseEvent& ev, void (ValueEditor::*event)(const RealPoint&, wxMouseEvent&)) {
	RealPoint pos = mousePoint(ev);
	// change viewer/editor
	ValueEditor* old_editor = current_editor;
	selectFieldNoEvents(pos);
	if (old_editor != current_editor) {
		// selection has changed, send focus events
		if (old_editor)     old_editor->onLoseFocus();
		if (current_editor) current_editor->onFocus();
	}
	// pass event
	if (current_editor) (current_editor->*event)(pos, ev);
	// refresh?
	if (old_editor != current_editor) {
		// selection has changed, refresh viewers
		// NOTE: after passing mouse down event, otherwise opening combo box produces flicker
		onChange();
	}
}
void DataEditor::selectFieldNoEvents(const RealPoint& p) {
	FOR_EACH_EDITOR_REVERSE { // find high z index fields first
		if (v->containsPoint(p) && v->getField()->editable) {
			current_viewer = v.get();
			current_editor = e;
			return;
		}
	}
	current_viewer = nullptr;
	current_editor = nullptr;
}

RealPoint DataEditor::mousePoint(const wxMouseEvent& ev) {
	StyleSheetP stylesheet = set->stylesheetFor(card);
	StyleSheetSettings& ss = settings.stylesheetSettingsFor(*stylesheet);
	Rotation rot(ss.card_angle(), stylesheet->getCardRect(), ss.card_zoom());
	return rot.trInv(RealPoint(ev.GetX(), ev.GetY()));
}

// ----------------------------------------------------------------------------- : Keyboard events

// ----------------------------------------------------------------------------- : Menu events

void DataEditor::onContextMenu(wxContextMenuEvent& ev) {
	if (current_editor) {
		IconMenu m;
		m.Append(wxID_CUT,	 _("TOOL_CUT"),		_("Cu&t"),		_("Move the selected text to the clipboard"));
		m.Append(wxID_COPY,	 _("TOOL_COPY"),	_("&Copy"),		_("Place the selected text on the clipboard"));
		m.Append(wxID_PASTE, _("TOOL_PASTE"),	_("&Paste"),	_("Inserts the text from the clipboard"));
		m.Enable(wxID_CUT,   canCut());
		m.Enable(wxID_COPY,  canCopy());
		m.Enable(wxID_PASTE, canPaste());
		if (current_editor->onContextMenu(m, ev)) {
			PopupMenu(&m);
		}
	}
}

// ----------------------------------------------------------------------------- : Focus events

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(DataEditor, CardViewer)
	EVT_LEFT_DOWN      (DataEditor::onLeftDown)
	EVT_LEFT_UP        (DataEditor::onLeftUp)
	EVT_LEFT_DCLICK    (DataEditor::onLeftDClick)
	EVT_RIGHT_DOWN     (DataEditor::onRightDown)
	EVT_MOTION         (DataEditor::onMotion)
	EVT_MOUSEWHEEL     (DataEditor::onMouseWheel)
	EVT_LEAVE_WINDOW   (DataEditor::onMouseLeave)
//	EVT_CONTEXT_MENU   (DataEditor::onContextMenu)
//	EVT_CHAR           (DataEditor::onChar)
//	EVT_SET_FOCUS      (DataEditor::onFocus)
//	EVT_KILL_FOCUS     (DataEditor::onLoseFocus)
//	EVT_MENU           (wxID_ANY, DataEditor::onMenu)
END_EVENT_TABLE  ()
