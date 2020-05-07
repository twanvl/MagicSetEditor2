//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/set_info_panel.hpp>
#include <gui/control/native_look_editor.hpp>
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
  add_tool_tr(tb, ID_FORMAT_BOLD, "bold", "bold", false, wxITEM_CHECK);
  add_tool_tr(tb, ID_FORMAT_ITALIC, "italic", "italic", false, wxITEM_CHECK);
  add_tool_tr(tb, ID_FORMAT_SYMBOL, "symbol", "symbols", false, wxITEM_CHECK);
  add_tool_tr(tb, ID_FORMAT_REMINDER, "reminder", "reminder_text", false, wxITEM_CHECK);
  tb->Realize();
  // Menus
  auto menuFormat = new wxMenu();
    add_menu_item_tr(menuFormat, ID_FORMAT_BOLD, "bold", "bold", wxITEM_CHECK);
    add_menu_item_tr(menuFormat, ID_FORMAT_ITALIC, "italic", "italic", wxITEM_CHECK);
    add_menu_item_tr(menuFormat, ID_FORMAT_SYMBOL, "symbol", "symbols", wxITEM_CHECK);
    add_menu_item_tr(menuFormat, ID_FORMAT_REMINDER, "reminder", "reminder_text", wxITEM_CHECK);
  mb->Insert(2, menuFormat, _MENU_("format"));
  // focus on editor
  editor->SetFocus();
}

void SetInfoPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
  // Toolbar
  tb->DeleteTool(ID_FORMAT_BOLD);
  tb->DeleteTool(ID_FORMAT_ITALIC);
  tb->DeleteTool(ID_FORMAT_SYMBOL);
  tb->DeleteTool(ID_FORMAT_REMINDER);
  // Menus
  delete mb->Remove(2);
}

void SetInfoPanel::onUpdateUI(wxUpdateUIEvent& ev) {
  switch (ev.GetId()) {
    case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: case ID_FORMAT_REMINDER: {
      ev.Enable(editor->canFormat(ev.GetId()));
      ev.Check (editor->hasFormat(ev.GetId()));
      break;
    }
  }
}

void SetInfoPanel::onCommand(int id) {
  switch (id) {
    case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC: case ID_FORMAT_SYMBOL: case ID_FORMAT_REMINDER: {
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
bool SetInfoPanel::canSelectAll() const { return editor->canSelectAll(); }
void SetInfoPanel::doSelectAll()        { editor->doSelectAll(); }
