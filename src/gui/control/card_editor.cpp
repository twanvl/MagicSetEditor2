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
#include <wx/caret.h>

DECLARE_TYPEOF_COLLECTION(ValueViewerP);
DECLARE_TYPEOF_COLLECTION(ValueViewer*);

// ----------------------------------------------------------------------------- : DataEditor

DataEditor::DataEditor(Window* parent, int id, long style)
	: CardViewer(parent, id, style)
	, current_viewer(nullptr)
	, current_editor(nullptr)
{
	// Create a caret
	SetCaret(new wxCaret(this,1,1));
}

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

void DataEditor::select(ValueViewer* v) {
	ValueEditor* old_editor = current_editor;
	current_viewer = v;
	current_editor = v->getEditor();
	if (current_editor != old_editor) {
		// selection has changed
		if (old_editor)     old_editor->onLoseFocus();
		if (current_editor) current_editor->onFocus();
		onChange();
	}
}

void DataEditor::selectFirst() {
	selectByTabPos(0);
}
bool DataEditor::selectNext() {
	return selectByTabPos(currentTabPos() + 1);
}
bool DataEditor::selectPrevious() {
	return selectByTabPos(currentTabPos() - 1);
}

bool DataEditor::selectByTabPos(int tab_pos) {
	if (tab_pos >= 0 && (size_t)tab_pos < by_tab_index.size()) {
		select(by_tab_index[tab_pos]);
		return true;
	} else if (!by_tab_index.empty()) {
		// also select something! so when we regain focus the selected editor makes sense
		if (tab_pos < 0) select(by_tab_index.back());
		else             select(by_tab_index.front());
	}
	return false;
}
int DataEditor::currentTabPos() const {
	int i = 0;
	FOR_EACH_CONST(v, by_tab_index) {
		if (v == current_viewer) return i;
		++i;
	}
	return -1;
}

struct CompareTabIndex {
	bool operator() (ValueViewer* a, ValueViewer* b) {
		Style& as = *a->getStyle(), &bs = *b->getStyle();
		Field& af = *as.fieldP,     &bf = *bs.fieldP;
		if (af.tab_index < bf.tab_index) return true;
		if (af.tab_index > bf.tab_index) return false;
		if (fabs(as.top - bs.top) < 15) {
			// the fields are almost on the same 'row'
			// compare horizontally first
			if (as.left < bs.left) return true; // horizontal sorting
			if (as.left > bs.left) return false;
			if (as.top  < bs.top)  return true; // vertical sorting
		} else {
			// compare vertically first
			if (as.top  < bs.top)  return true; // vertical sorting
			if (as.top  > bs.top)  return false;
			if (as.left < bs.left) return true; // horizontal sorting
		}
		return false;
	}
};
void DataEditor::createTabIndex() {
	by_tab_index.clear();
	FOR_EACH(v, viewers) {
		if (v->getField()->editable && v->getStyle()->visible) {
			by_tab_index.push_back(v.get());
		}
	}
	stable_sort(by_tab_index.begin(), by_tab_index.end(), CompareTabIndex());
}
void DataEditor::onInit() {
	createTabIndex();
	current_viewer = nullptr;
	current_editor = nullptr;
}

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


wxMenu* DataEditor::getMenu(int type) const {
	if (current_editor) {
		return current_editor->getMenu(type);
	} else {
		return nullptr;
	}
}
void DataEditor::onCommand(int id) {
	if (current_editor) {
		current_editor->onCommand(id);
	}
}

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

void DataEditor::onChar(wxKeyEvent& ev) {
	if (ev.GetKeyCode() == WXK_TAB) {
		if (!ev.ShiftDown()) {
			// try to select the next editor
			if (selectNext()) return;
			// send a navigation event to our parent, to select another control
			wxNavigationKeyEvent evt;
			GetParent()->ProcessEvent(evt);
		} else {
			// try to select the previos editor
			if (selectPrevious()) return;
			// send a navigation event to our parent, to select another control
			wxNavigationKeyEvent evt;
			evt.SetDirection(false);
			GetParent()->ProcessEvent(evt);
		}
	} else if (current_editor) {
		current_editor->onChar(ev);
	}
}

// ----------------------------------------------------------------------------- : Menu events

void DataEditor::onContextMenu(wxContextMenuEvent& ev) {
	if (current_editor) {
		IconMenu m;
		m.Append(wxID_CUT,	 _("cut"),		_("Cu&t"),		_("Move the selected text to the clipboard"));
		m.Append(wxID_COPY,	 _("copy"),	_("&Copy"),		_("Place the selected text on the clipboard"));
		m.Append(wxID_PASTE, _("paste"),	_("&Paste"),	_("Inserts the text from the clipboard"));
		m.Enable(wxID_CUT,   canCut());
		m.Enable(wxID_COPY,  canCopy());
		m.Enable(wxID_PASTE, canPaste());
		if (current_editor->onContextMenu(m, ev)) {
			PopupMenu(&m);
		}
	}
}
void DataEditor::onMenu(wxCommandEvent& ev) {
	if (current_editor) {
		if (!current_editor->onCommand(ev.GetId())) {
			ev.Skip();
		}
	} else {
		ev.Skip();
	}
}

// ----------------------------------------------------------------------------- : Focus events

void DataEditor::onFocus(wxFocusEvent& ev) {
	if (current_editor) {
		current_editor->onFocus();
		onChange();
	}
}
void DataEditor::onLoseFocus(wxFocusEvent& ev) {
	if (current_editor) {
		current_editor->onLoseFocus();
		onChange();
	}
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(DataEditor, CardViewer)
	EVT_LEFT_DOWN      (DataEditor::onLeftDown)
	EVT_LEFT_UP        (DataEditor::onLeftUp)
	EVT_LEFT_DCLICK    (DataEditor::onLeftDClick)
	EVT_RIGHT_DOWN     (DataEditor::onRightDown)
	EVT_MOTION         (DataEditor::onMotion)
	EVT_MOUSEWHEEL     (DataEditor::onMouseWheel)
	EVT_LEAVE_WINDOW   (DataEditor::onMouseLeave)
	EVT_CONTEXT_MENU   (DataEditor::onContextMenu)
	EVT_MENU           (wxID_ANY, DataEditor::onMenu)
	EVT_CHAR           (DataEditor::onChar)
	EVT_SET_FOCUS      (DataEditor::onFocus)
	EVT_KILL_FOCUS     (DataEditor::onLoseFocus)
END_EVENT_TABLE  ()
