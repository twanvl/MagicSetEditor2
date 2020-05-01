//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/window.hpp>
#include <gui/symbol/control.hpp>
#include <gui/symbol/part_list.hpp>
#include <gui/util.hpp>
#include <data/field/symbol.hpp>
#include <data/format/image_to_symbol.hpp>
#include <data/action/value.hpp>
#include <data/set.hpp> // :(
#include <util/window_id.hpp>
#include <util/io/reader.hpp>
#include <util/io/package.hpp>
#include <util/error.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/spinctrl.h>

// ----------------------------------------------------------------------------- : Constructor

SymbolWindow::SymbolWindow(Window* parent)
  : performer(nullptr)
{
  init(parent, default_symbol());
}

SymbolWindow::SymbolWindow(Window* parent, const String& filename)
  : performer(nullptr)
{
  // open file
  wxFileInputStream stream(filename);
  Reader reader(stream, nullptr, filename);
  SymbolP symbol;
  reader.handle_greedy(symbol);
  init(parent, symbol);
}

SymbolWindow::SymbolWindow(Window* parent, ValueActionPerformer* performer)
  : performer(performer)
{
  // attempt to load symbol
  SymbolP symbol;
  SymbolValueP value = static_pointer_cast<SymbolValue>(performer->value);
  if (!value->filename.empty()) {
    try {
      // load symbol
      Package& package = performer->getLocalPackage();
      symbol = package.readFile<SymbolP>(value->filename);
    } catch (const Error& e) {
      handle_error(e);
    }
  }
  if (!symbol) symbol = default_symbol();
  init(parent, symbol);
}
SymbolWindow::~SymbolWindow() {
  delete performer;
}

void SymbolWindow::init(Window* parent, SymbolP symbol) {
  Create(parent, wxID_ANY, _TITLE_("symbol editor"), wxDefaultPosition, wxSize(650,600), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
  SetIcon(load_resource_icon(_("app")));
  inSelectionEvent = false;
  
  // Menu bar
  auto menuBar = new wxMenuBar();
  auto menuFile = new wxMenu();
    add_menu_item_tr(menuFile, ID_FILE_NEW, "new", "new_symbol");
    add_menu_item_tr(menuFile, ID_FILE_OPEN, "open", "open_symbol");
    add_menu_item_tr(menuFile, ID_FILE_SAVE, "save", "save_symbol");
    add_menu_item_tr(menuFile, ID_FILE_SAVE_AS, nullptr, "save_symbol_as");
    menuFile->AppendSeparator();
    add_menu_item_tr(menuFile, ID_FILE_STORE, "apply", "store_symbol");
    menuFile->AppendSeparator();
    add_menu_item_tr(menuFile, ID_FILE_EXIT, nullptr, "close_symbol_editor");
  menuBar->Append(menuFile, _MENU_("file"));
  
  auto menuEdit = new wxMenu();
    add_menu_item(menuEdit, ID_EDIT_UNDO, "undo", _MENU_1_("undo",wxEmptyString), _HELP_("undo"));
    add_menu_item(menuEdit, ID_EDIT_REDO, "redo", _MENU_1_("redo",wxEmptyString), _HELP_("redo"));
    menuEdit->AppendSeparator();
    add_menu_item_tr(menuEdit, ID_EDIT_GROUP, "group", "group");
    add_menu_item_tr(menuEdit, ID_EDIT_UNGROUP, "ungroup", "ungroup");
    menuEdit->AppendSeparator();
    add_menu_item_tr(menuEdit, ID_EDIT_DUPLICATE, "duplicate", "duplicate");
  menuBar->Append(menuEdit, _MENU_("edit"));
  
  auto menuTool = new wxMenu();
    add_menu_item_tr(menuTool, ID_MODE_SELECT, "mode_select", "select", wxITEM_CHECK);
    add_menu_item_tr(menuTool, ID_MODE_ROTATE, "mode_rotate", "rotate", wxITEM_CHECK);
    add_menu_item_tr(menuTool, ID_MODE_POINTS, "mode_curve", "points", wxITEM_CHECK);
    add_menu_item_tr(menuTool, ID_MODE_SHAPES, "circle", "basic_shapes", wxITEM_CHECK);
    add_menu_item_tr(menuTool, ID_MODE_SYMMETRY, "mode_symmetry", "symmetry", wxITEM_CHECK);
    add_menu_item_tr(menuTool, ID_MODE_PAINT, "mode_paint", "paint", wxITEM_CHECK);
  menuBar->Append(menuTool, _MENU_("tool"));
  
  SetMenuBar(menuBar);
  
  // Statusbar
  CreateStatusBar();
  SetStatusText(_(""));
  
  // Toolbar
  wxToolBar* tb = CreateToolBar(wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL | wxTB_TEXT);
  add_tool_tr(tb, ID_FILE_STORE, "apply", "store_symbol", true);
  tb->AddSeparator();
  add_tool(tb, ID_EDIT_UNDO, "undo", _TOOL_("undo"), _TOOLTIP_1_("undo",wxEmptyString), _HELP_("undo"));
  add_tool(tb, ID_EDIT_REDO, "redo", _TOOL_("redo"), _TOOLTIP_1_("redo",wxEmptyString), _HELP_("redo"));
  tb->AddSeparator();
  add_tool_tr(tb, ID_VIEW_GRID, "grid", "grid", true, wxITEM_CHECK);
  add_tool_tr(tb, ID_VIEW_GRID_SNAP, "grid_snap", "snap", true, wxITEM_CHECK);
  tb->Realize();
  
  // Edit mode toolbar
  wxPanel* emp = new wxPanel(this, wxID_ANY);
  wxToolBar* em = new wxToolBar(emp, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_VERTICAL | wxTB_HORZ_TEXT);
  em->SetToolBitmapSize(wxSize(17,17));
  add_tool_tr(em, ID_MODE_SELECT, "mode_select", "select", true, wxITEM_CHECK);
  add_tool_tr(em, ID_MODE_ROTATE, "mode_rotate", "rotate", true, wxITEM_CHECK);
  em->AddSeparator();
  add_tool_tr(em, ID_MODE_POINTS, "mode_curve", "points", true, wxITEM_CHECK);
  em->AddSeparator();
  add_tool_tr(em, ID_MODE_SHAPES, "circle", "basic shapes", true, wxITEM_CHECK);
  add_tool_tr(em, ID_MODE_SYMMETRY, "mode_symmetry", "symmetry", true, wxITEM_CHECK);
  //add_tool_tr(em, ID_MODE_PAINT, "mode_paint", "paint", true, wxITEM_CHECK);
  em->Realize();
  
  // Lay out
  wxSizer* es = new wxBoxSizer(wxVERTICAL);
  es->Add(em, 1, wxEXPAND | wxBOTTOM, 5);
  emp->SetSizer(es);
  
  // Controls
  control = new SymbolControl (this, ID_CONTROL, symbol);
  parts   = new SymbolPartList(this, ID_PART_LIST, control->selected_parts, symbol);
  
  wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
    wxSizer* v = new wxBoxSizer(wxVERTICAL);
    v->Add(emp,   0, wxEXPAND);
    v->Add(parts, 1, wxEXPAND);
  s->Add(v,       0, wxEXPAND);
  s->Add(control, 1, wxEXPAND);
  SetSizer(s);
  
  #ifdef __WXMSW__
    // only tested on msw, may not even be needed on other platforms
    #define USE_HORRIBLE_HORRIBLE_HACK_TO_MAKE_TOOLBAR_THE_RIGHT_SIZE
  #endif
  
  #ifdef USE_HORRIBLE_HORRIBLE_HACK_TO_MAKE_TOOLBAR_THE_RIGHT_SIZE
    // Welcome to HackWorld
    
    // Prevent clipping of the bottom tool
    Layout();
    em->SetSize(emp->GetSize());
    
    // HACK: force edit mode toolbar to be wide enough by adding spaces to tool names
    int n = 0;
    while (em->GetSize().x + 5 < emp->GetSize().x) {
      ++n;
      for (int id = ID_MODE_SELECT ; id <= ID_MODE_SYMMETRY ; ++id) {
        wxToolBarToolBase* tool = em->FindById(id);
        tool->SetLabel(tool->GetLabel() + _(" "));
      }
      em->Realize();
    }
    // And now rebuild it, because the above meddling broke the toolbar for some unknown reason
    em->Destroy();
    em = new wxToolBar(emp, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_VERTICAL | wxTB_HORZ_TEXT);
    em->SetToolBitmapSize(wxSize(17,17));
    String spaces(max(0,n-1), _(' '));
    add_tool(em, ID_MODE_SELECT,   "mode_select",  _TOOL_("select")       + spaces, _TOOLTIP_("select"),    _HELP_("select"), wxITEM_CHECK);
    add_tool(em, ID_MODE_ROTATE,   "mode_rotate",  _TOOL_("rotate")       + spaces, _TOOLTIP_("rotate"),    _HELP_("rotate"), wxITEM_CHECK);
    em->AddSeparator();
    add_tool(em, ID_MODE_POINTS,   "mode_curve",   _TOOL_("points")       + spaces, _TOOLTIP_("points"),    _HELP_("points"), wxITEM_CHECK);
    em->AddSeparator();
    add_tool(em, ID_MODE_SHAPES,   "circle",        _TOOL_("basic shapes") + spaces, _TOOLTIP_("basic shapes"),_HELP_("basic shapes"), wxITEM_CHECK);
    add_tool(em, ID_MODE_SYMMETRY, "mode_symmetry", _TOOL_("symmetry")     + spaces, _TOOLTIP_("symmetry"),  _HELP_("symmetry"), wxITEM_CHECK);
    em->Realize();
    
    es = new wxBoxSizer(wxVERTICAL);
    es->Add(em, 1, wxEXPAND | wxBOTTOM, 5);
    emp->SetSizer(es);
    
    // Prevent clipping of the bottom tool
    Layout();
    em->SetSize(emp->GetSize());
  #endif
  
  // we want update ui events
  wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);
  SetExtraStyle(wxWS_EX_PROCESS_UI_UPDATES);
  em->SetExtraStyle(wxWS_EX_PROCESS_UI_UPDATES);
}

// ----------------------------------------------------------------------------- : Event handling

void SymbolWindow::onFileNew(wxCommandEvent& ev) {
  SymbolP symbol = default_symbol();
  parts->setSymbol(symbol);
  control->setSymbol(symbol);
}

void SymbolWindow::onFileOpen(wxCommandEvent& ev) {
  String name = wxFileSelector(_("Open symbol"),settings.default_symbol_dir,_(""),_(""),_("Symbol files|*.mse-symbol;*.bmp|MSE2 symbol files (*.mse-symbol)|*.mse-symbol|Images/MSE1 symbol files|*.bmp;*.png;*.jpg;*.gif"),wxFD_OPEN|wxFD_FILE_MUST_EXIST, this);
  if (!name.empty()) {
    settings.default_symbol_dir = wxPathOnly(name);
    wxFileName n(name);
    String ext = n.GetExt();
    SymbolP symbol;
    if (ext.Lower() == _("mse-symbol")) {
      wxFileInputStream stream(name);
      Reader reader(stream, nullptr, name);
      reader.handle_greedy(symbol);
    } else {
      wxBusyCursor busy;
      Image image(name);
      if (!image.Ok()) return;
      symbol = import_symbol(image);
    }
    // show...
    parts->setSymbol(symbol);
    control->setSymbol(symbol);
  }
}

void SymbolWindow::onFileSave(wxCommandEvent& ev) {
  // TODO
  onFileSaveAs(ev);
}

void SymbolWindow::onFileSaveAs(wxCommandEvent& ev) {
  String name = wxFileSelector(_("Save symbol"),settings.default_set_dir,_(""),_(""),_("Symbol files (*.mse-symbol)|*.mse-symbol"),wxFD_SAVE, this);
  if (!name.empty()) {
    settings.default_set_dir = wxPathOnly(name);
    wxFileOutputStream stream(name);
    Writer writer(stream, file_version_symbol);
    writer.handle(control->getSymbol());
  }
}

void SymbolWindow::onFileStore(wxCommandEvent& ev) {
  if (performer) {
    SymbolValueP value = static_pointer_cast<SymbolValue>(performer->value);
    Package& package = performer->getLocalPackage();
    LocalFileName new_filename = package.newFileName(value->field().name,_(".mse-symbol")); // a new unique name in the package
    auto stream = package.openOut(new_filename);
    Writer writer(*stream, file_version_symbol);
    writer.handle(control->getSymbol());
    performer->addAction(value_action(value, new_filename));
  }
}

void SymbolWindow::onFileExit(wxCommandEvent& ev) {
  Close();
}


void SymbolWindow::onEditUndo(wxCommandEvent& ev) {
  if (!control->isEditing() && control->getSymbol()->actions.canUndo()) {
    control->getSymbol()->actions.undo();
    control->Refresh(false);
  }
}

void SymbolWindow::onEditRedo(wxCommandEvent& ev) {
  if (!control->isEditing() && control->getSymbol()->actions.canRedo()) {
    control->getSymbol()->actions.redo();
    control->Refresh(false);
  }
}

void SymbolWindow::onModeChange(wxCommandEvent& ev) {
  control->onModeChange(ev);
}

void SymbolWindow::onExtraTool(wxCommandEvent& ev) {
  control->onExtraTool(ev);
}


void SymbolWindow::onUpdateUI(wxUpdateUIEvent& ev) {
  switch(ev.GetId()) {
    // file menu
    case ID_FILE_STORE: {
      ev.Enable(performer);
      break;
    // undo/redo
    } case ID_EDIT_UNDO: {
      ev.Enable(control->getSymbol()->actions.canUndo());
      String label = control->getSymbol()->actions.undoName();
      ev.SetText(_MENU_1_("undo", label));
      GetToolBar()->SetToolShortHelp(ID_EDIT_UNDO, _TOOLTIP_1_("undo", label));
      break;
    } case ID_EDIT_REDO: {
      ev.Enable(control->getSymbol()->actions.canRedo());
      String label = control->getSymbol()->actions.redoName();
      ev.SetText(_MENU_1_("redo", label));
      GetToolBar()->SetToolShortHelp(ID_EDIT_REDO, _TOOLTIP_1_("redo", label));
      break;
    } default: {
      // items created by the editor control
      control->onUpdateUI(ev);
      break;
    }
  }
}


void SymbolWindow::onSelectFromList(wxCommandEvent& ev) {
  if (inSelectionEvent) return;
  inSelectionEvent = true;
  control->onUpdateSelection();
  inSelectionEvent = false;
}
void SymbolWindow::onActivateFromList(wxCommandEvent& ev) {
  if (control->selected_parts.size() == 1) {
    control->activatePart(control->selected_parts.getOnlyOne());
  }
}

void SymbolWindow::onSelectFromControl() {
  if (inSelectionEvent) return ;
  inSelectionEvent = true;
  parts->Refresh(false);
  inSelectionEvent = false;
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(SymbolWindow, wxFrame)
  EVT_MENU    (ID_FILE_NEW,    SymbolWindow::onFileNew)
  EVT_MENU    (ID_FILE_OPEN,    SymbolWindow::onFileOpen)
  EVT_MENU    (ID_FILE_SAVE,    SymbolWindow::onFileSave)
  EVT_MENU    (ID_FILE_SAVE_AS,  SymbolWindow::onFileSaveAs)
  EVT_MENU    (ID_FILE_STORE,    SymbolWindow::onFileStore)
  EVT_MENU    (ID_FILE_EXIT,    SymbolWindow::onFileExit)
  EVT_MENU    (ID_EDIT_UNDO,    SymbolWindow::onEditUndo)
  EVT_MENU    (ID_EDIT_REDO,    SymbolWindow::onEditRedo)

  EVT_TOOL_RANGE  (ID_MODE_MIN,  ID_MODE_MAX,    SymbolWindow::onModeChange)
  EVT_TOOL_RANGE  (ID_CHILD_MIN, ID_CHILD_MAX,  SymbolWindow::onExtraTool)
  EVT_UPDATE_UI  (wxID_ANY,            SymbolWindow::onUpdateUI)
  EVT_COMMAND_RANGE(ID_CHILD_MIN, ID_CHILD_MAX, wxEVT_SPINCTRL, SymbolWindow::onExtraTool)

  EVT_PART_SELECT   (ID_PART_LIST, SymbolWindow::onSelectFromList)
  EVT_PART_ACTIVATE (ID_PART_LIST, SymbolWindow::onActivateFromList)
END_EVENT_TABLE  ()
