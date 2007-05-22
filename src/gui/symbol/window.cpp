//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/window.hpp>
#include <gui/symbol/control.hpp>
#include <gui/symbol/part_list.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <data/set.hpp>
#include <data/field/symbol.hpp>
#include <data/format/image_to_symbol.hpp>
#include <data/action/value.hpp>
#include <util/window_id.hpp>
#include <util/io/reader.hpp>
#include <util/error.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>

// ----------------------------------------------------------------------------- : Constructor

SymbolWindow::SymbolWindow(Window* parent) {
	init(parent, default_symbol());
}

SymbolWindow::SymbolWindow(Window* parent, const String& filename) {
	// open file
	Reader reader(filename);
	SymbolP symbol;
	reader.handle_greedy(symbol);
	init(parent, symbol);
}

SymbolWindow::SymbolWindow(Window* parent, const SymbolValueP& value, const SetP& set)
	: value(value), set(set)
{
	// attempt to load symbol
	SymbolP symbol;
	if (!value->filename.empty()) {
		try {
			// load symbol
			symbol = set->readFile<SymbolP>(value->filename);
		} catch (const Error& e) {
			handle_error(e);
		}
	}
	if (!symbol) symbol = default_symbol();
	init(parent, symbol);
}

void SymbolWindow::init(Window* parent, SymbolP symbol) {
	Create(parent, wxID_ANY, _TITLE_("symbol editor"), wxDefaultPosition, wxSize(900,900), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
	SetIcon(load_resource_icon(_("app")));
	inSelectionEvent = false;
	
	// Menu bar
	wxMenuBar* menuBar = new wxMenuBar();
	IconMenu* menuFile = new IconMenu();
		menuFile->Append(ID_FILE_NEW,		_("new"),			_MENU_("new symbol"),			_HELP_("new symbol"));
		menuFile->Append(ID_FILE_OPEN,		_("open"),			_MENU_("open symbol"),			_HELP_("open symbol"));
		menuFile->Append(ID_FILE_SAVE,		_("save"),			_MENU_("save symbol"),			_HELP_("save symbol"));
		menuFile->Append(ID_FILE_SAVE_AS,						_MENU_("save symbol as"),		_HELP_("save symbol as"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_STORE,		_("apply"),			_MENU_("store symbol"),			_HELP_("store symbol"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_EXIT,							_MENU_("close symbol editor"),	_HELP_("close symbol editor"));
	menuBar->Append(menuFile, _MENU_("file"));
	
	IconMenu* menuEdit = new IconMenu();
		menuEdit->Append(ID_EDIT_UNDO,		_("undo"),			_MENU_1_("undo",wxEmptyString),	_HELP_("undo"));
		menuEdit->Append(ID_EDIT_REDO,		_("redo"),			_MENU_1_("redo",wxEmptyString),	_HELP_("redo"));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_DUPLICATE,	_("duplicate"),		_MENU_("duplicate"),			_HELP_("duplicate"));
	menuBar->Append(menuEdit, _MENU_("edit"));
	
	IconMenu* menuTool = new IconMenu();
		menuTool->Append(ID_MODE_SELECT,	_("mode_select"),	_MENU_("select"),				_HELP_("select"),		wxITEM_CHECK);
		menuTool->Append(ID_MODE_ROTATE,	_("mode_rotate"),	_MENU_("rotate"),				_HELP_("rotate"),		wxITEM_CHECK);
		menuTool->Append(ID_MODE_POINTS,	_("mode_curve"),	_MENU_("points"),				_HELP_("points"),		wxITEM_CHECK);
		menuTool->Append(ID_MODE_SHAPES,	_("circle"),		_MENU_("basic shapes"),			_HELP_("basic shapes"),	wxITEM_CHECK);
		menuTool->Append(ID_MODE_PAINT,		_("mode_paint"),	_MENU_("paint"),				_HELP_("paint"),		wxITEM_CHECK);
	menuBar->Append(menuTool, _MENU_("tool"));
	
	SetMenuBar(menuBar);
	
	// Statusbar
	CreateStatusBar();
	SetStatusText(_(""));
	
	// Toolbar
	wxToolBar* tb = CreateToolBar(wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL | wxTB_TEXT);
	tb->AddTool(ID_FILE_STORE,		_TOOL_("store symbol"),	load_resource_tool_image(_("apply")),		wxNullBitmap, wxITEM_NORMAL, _TOOLTIP_("store symbol"), _HELP_("store symbol"));
	tb->AddSeparator();
	tb->AddTool(ID_EDIT_UNDO,		_TOOL_("undo"),			load_resource_tool_image(_("undo")),		wxNullBitmap, wxITEM_NORMAL, _TOOLTIP_1_("undo",wxEmptyString));
	tb->AddTool(ID_EDIT_REDO,		_TOOL_("redo"),			load_resource_tool_image(_("redo")),		wxNullBitmap, wxITEM_NORMAL, _TOOLTIP_1_("redo",wxEmptyString));
	tb->AddSeparator();
	tb->AddTool(ID_VIEW_GRID,		_TOOL_("grid"),			load_resource_tool_image(_("grid")),		wxNullBitmap, wxITEM_CHECK,  _TOOLTIP_("grid"),			_HELP_("grid"));
	tb->AddTool(ID_VIEW_GRID_SNAP,	_TOOL_("snap"),			load_resource_tool_image(_("grid_snap")),	wxNullBitmap, wxITEM_CHECK,  _TOOLTIP_("snap"),			_HELP_("snap"));
	tb->Realize();
	
	// Edit mode toolbar
	wxPanel* emp = new wxPanel(this, wxID_ANY);
	wxToolBar* em = new wxToolBar(emp, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_VERTICAL | wxTB_TEXT | wxTB_HORZ_LAYOUT);
	em->AddTool(ID_MODE_SELECT,		_TOOL_("select"),		load_resource_tool_image(_("mode_select")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("select"),		_HELP_("select"));
	em->AddTool(ID_MODE_ROTATE,		_TOOL_("rotate"),		load_resource_tool_image(_("mode_rotate")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("rotate"),		_HELP_("rotate"));
	em->AddSeparator();
	em->AddTool(ID_MODE_POINTS,		_TOOL_("points"),		load_resource_tool_image(_("mode_curve")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("points"),		_HELP_("points"));
	em->AddSeparator();
	em->AddTool(ID_MODE_SHAPES,		_TOOL_("basic shapes"),	load_resource_tool_image(_("circle")),		wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("basic shapes"),	_HELP_("basic shapes"));
	em->AddSeparator();
	em->AddTool(ID_MODE_PAINT,		_TOOL_("paint"),		load_resource_tool_image(_("mode_paint")),	wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("paint"),			_HELP_("paint"));
	em->AddSeparator();
	em->Realize();
	
	// Controls
	control = new SymbolControl (this, ID_CONTROL, symbol);
	parts   = new SymbolPartList(this, ID_PART_LIST, symbol);
	
	// Lay out
	wxSizer* es = new wxBoxSizer(wxHORIZONTAL);
	es->Add(em, 0, wxEXPAND | wxTOP | wxBOTTOM | wxALIGN_CENTER, 1);
	emp->SetSizer(es);
	
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		wxSizer* v = new wxBoxSizer(wxVERTICAL);
		v->Add(emp,   0, wxEXPAND);
		v->Add(parts, 1, wxEXPAND);
	s->Add(v,       0, wxEXPAND);
	s->Add(control, 1, wxEXPAND);
	SetSizer(s);
	
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
	String name = wxFileSelector(_("Open symbol"),_(""),_(""),_(""),_("Symbol files|*.mse-symbol;*.bmp|MSE2 symbol files (*.mse-symbol)|*.mse-symbol|Images/MSE1 symbol files|*.bmp;*.png;*.jpg;*.gif"),wxOPEN|wxFILE_MUST_EXIST, this);
	if (!name.empty()) {
		wxFileName n(name);
		String ext = n.GetExt();
		SymbolP symbol;
		if (ext.Lower() == _("mse-symbol")) {
			Reader reader(new_shared1<wxFileInputStream>(name), name);
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
	String name = wxFileSelector(_("Save symbol"),_(""),_(""),_(""),_("Symbol files (*.mse-symbol)|*.mse-symbol"),wxSAVE, this);
	if (!name.empty()) {
		Writer writer(new_shared1<wxFileOutputStream>(name));
		writer.handle(control->getSymbol());
	}
}

void SymbolWindow::onFileStore(wxCommandEvent& ev) {
	if (value) {
		FileName new_filename = set->newFileName(value->field().name,_(".mse-symbol")); // a new unique name in the package
		Writer writer(set->openOut(new_filename));
		writer.handle(control->getSymbol());
		set->actions.add(value_action(value, new_filename));
	}
}

void SymbolWindow::onFileExit(wxCommandEvent& ev) {
	Close();
}


void SymbolWindow::onEditUndo(wxCommandEvent& ev) {
	if (!control->isEditing()) {
		control->getSymbol()->actions.undo();
		control->Refresh(false);
	}
}

void SymbolWindow::onEditRedo(wxCommandEvent& ev) {
	if (!control->isEditing()) {
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
			ev.Enable(value);
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


void SymbolWindow::onSelectFromList(wxListEvent& ev) {
	if (inSelectionEvent) return ;
	inSelectionEvent = true;
	parts->getselected_parts(control->selected_parts);
	control->onUpdateSelection();
	inSelectionEvent = false;
}
void SymbolWindow::onActivateFromList(wxListEvent& ev) {
	control->activatePart(control->getSymbol()->parts.at(ev.GetIndex()));
}

void SymbolWindow::onSelectFromControl() {
	if (inSelectionEvent) return ;
	inSelectionEvent = true;
	parts->selectParts(control->selected_parts);
	inSelectionEvent = false;
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(SymbolWindow, wxFrame)
	EVT_MENU		(ID_FILE_NEW,		SymbolWindow::onFileNew)
	EVT_MENU		(ID_FILE_OPEN,		SymbolWindow::onFileOpen)
	EVT_MENU		(ID_FILE_SAVE,		SymbolWindow::onFileSave)
	EVT_MENU		(ID_FILE_SAVE_AS,	SymbolWindow::onFileSaveAs)
	EVT_MENU		(ID_FILE_STORE,		SymbolWindow::onFileStore)
	EVT_MENU		(ID_FILE_EXIT,		SymbolWindow::onFileExit)
	EVT_MENU		(ID_EDIT_UNDO,		SymbolWindow::onEditUndo)
	EVT_MENU		(ID_EDIT_REDO,		SymbolWindow::onEditRedo)

	EVT_TOOL_RANGE	(ID_MODE_MIN,  ID_MODE_MAX,		SymbolWindow::onModeChange)
	EVT_TOOL_RANGE	(ID_CHILD_MIN, ID_CHILD_MAX,	SymbolWindow::onExtraTool)
	EVT_UPDATE_UI	(wxID_ANY,						SymbolWindow::onUpdateUI)

	EVT_LIST_ITEM_SELECTED   (ID_PART_LIST, SymbolWindow::onSelectFromList)
	EVT_LIST_ITEM_DESELECTED (ID_PART_LIST, SymbolWindow::onSelectFromList)
	EVT_LIST_ITEM_ACTIVATED  (ID_PART_LIST, SymbolWindow::onActivateFromList)
END_EVENT_TABLE  ()
