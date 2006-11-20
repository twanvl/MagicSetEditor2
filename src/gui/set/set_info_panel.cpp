//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/set_info_panel.hpp>
#include <gui/control/native_look_editor.hpp>
#include <gui/icon_menu.hpp>
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
	tb->AddTool(ID_FORMAT_BOLD,		_(""), Bitmap(_("TOOL_BOLD")),		wxNullBitmap, wxITEM_CHECK, _("Bold"));
	tb->AddTool(ID_FORMAT_ITALIC,	_(""), Bitmap(_("TOOL_ITALIC")),	wxNullBitmap, wxITEM_CHECK, _("Italic"));
	tb->AddTool(ID_FORMAT_SYMBOL,	_(""), Bitmap(_("TOOL_SYMBOL")),	wxNullBitmap, wxITEM_CHECK, _("Symbols"));
	tb->Realize();
	// Menus
	IconMenu* menuFormat = new IconMenu();
		menuFormat->Append(ID_FORMAT_BOLD,		_("TOOL_BOLD"),		_("Bold\tCtrl+B"),					_("Makes the selected text bold"),			wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_ITALIC,	_("TOOL_ITALIC"),	_("Italic\tCtrl+I"),				_("Makes the selected text italic"),		wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_SYMBOL,	_("TOOL_SYMBOL"),	_("Symbols\tCtrl+M"),				_("Draws the selected text with symbols"),	wxITEM_CHECK);
		menuFormat->Append(ID_FORMAT_REMINDER,	_("TOOL_REMINDER"),	_("Reminder Text\tCtrl+R"),			_("Show reminder text for the selected keyword"),	wxITEM_CHECK);
	mb->Insert(2, menuFormat, _("&Format"));
}

void SetInfoPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->DeleteTool(ID_FORMAT_BOLD);
	tb->DeleteTool(ID_FORMAT_ITALIC);
	tb->DeleteTool(ID_FORMAT_SYMBOL);
	// Menus
	delete mb->Remove(2);
}

void SetInfoPanel::onUpdateUI(wxUpdateUIEvent& e) {
	switch (e.GetId()) {
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: {
			e.Enable(editor->canFormat(e.GetId()));
			e.Check (editor->hasFormat(e.GetId()));
			break;
		}
	}
}

void SetInfoPanel::onCommand(int id) {
	switch (id) {
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: {
			editor->doFormat(id);
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
