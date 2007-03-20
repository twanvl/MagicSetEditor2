//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/set_info_panel.hpp>
#include <gui/control/native_look_editor.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>

// ----------------------------------------------------------------------------- : SetInfoPanel

SetInfoPanel::SetInfoPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	editor = new SetInfoEditor(this, wxID_ANY);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(editor, 1, wxEXPAND, 2);
	s->SetSizeHints(this);
	SetSizer(s);
}

void SetInfoPanel::onChangeSet() {
	editor->setSet(set);
}

// ----------------------------------------------------------------------------- : UI

void SetInfoPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->AddTool(ID_FORMAT_BOLD,		_(""), load_resource_tool_image(_("bold")),		wxNullBitmap, wxITEM_CHECK, _TOOL_("bold"));
	tb->AddTool(ID_FORMAT_ITALIC,	_(""), load_resource_tool_image(_("italic")),	wxNullBitmap, wxITEM_CHECK, _TOOL_("italic"));
	tb->AddTool(ID_FORMAT_SYMBOL,	_(""), load_resource_tool_image(_("symbol")),	wxNullBitmap, wxITEM_CHECK, _TOOL_("symbols"));
	tb->Realize();
	// Menus
	IconMenu* menuFormat = new IconMenu();
		menuFormat->Append(ID_FORMAT_BOLD,		_("bold"),			_MENU_("bold"),				_HELP_("bold"),				wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_ITALIC,	_("italic"),		_MENU_("italic"),			_HELP_("italic"),			wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_SYMBOL,	_("symbol"),		_MENU_("symbols"),			_HELP_("symbols"),			wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_REMINDER,	_("reminder"),		_MENU_("reminder text"),	_HELP_("reminder text"),	wxITEM_CHECK);
	mb->Insert(2, menuFormat, _MENU_("format"));
}

void SetInfoPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->DeleteTool(ID_FORMAT_BOLD);
	tb->DeleteTool(ID_FORMAT_ITALIC);
	tb->DeleteTool(ID_FORMAT_SYMBOL);
	// Menus
	delete mb->Remove(2);
}

void SetInfoPanel::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: {
			ev.Enable(editor->canFormat(ev.GetId()));
			ev.Check (editor->hasFormat(ev.GetId()));
			break;
		}
	}
}

void SetInfoPanel::onCommand(int id) {
	switch (id) {
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: {
			editor->doFormat(id);
			break;
		}
	}
}

// ----------------------------------------------------------------------------- : Clipboard

bool SetInfoPanel::canCut()   const { return editor->canCut();   }
bool SetInfoPanel::canCopy()  const { return editor->canCopy();  }
bool SetInfoPanel::canPaste() const { return editor->canPaste(); }
void SetInfoPanel::doCut()          {        editor->doCut();    }
void SetInfoPanel::doCopy()         {        editor->doCopy();   }
void SetInfoPanel::doPaste()        {        editor->doPaste();  }
