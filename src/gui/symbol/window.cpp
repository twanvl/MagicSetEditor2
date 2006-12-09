//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/window.hpp>
#include <gui/symbol/control.hpp>
#include <gui/symbol/part_list.hpp>
#include <gui/icon_menu.hpp>
#include <data/set.hpp>
#include <data/field/symbol.hpp>
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
	// TODO : open file
	init(parent, default_symbol());
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
	Create(parent, wxID_ANY, _("Symbol Editor"), wxDefaultPosition, wxSize(600,600), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
	inSelectionEvent = false;
	
	// Menu bar
	wxMenuBar* menuBar = new wxMenuBar();
	IconMenu* menuFile = new IconMenu();
		menuFile->Append(ID_FILE_NEW,		_("TOOL_NEW"),		_("&New...\tCtrl+N"),		_("Create a new symbol"));
		menuFile->Append(ID_FILE_OPEN,		_("TOOL_OPEN"),		_("&Open...\tCtrl+O"),		_("Open a symbol"));
		menuFile->Append(ID_FILE_SAVE,		_("TOOL_SAVE"),		_("&Save\tCtrl+S"),			_("Save the symbol"));
		menuFile->Append(ID_FILE_SAVE_AS,						_("Save &As...\tF12"),		_("Save the symbol under a diferent filename"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_STORE,		_("TOOL_APPLY"),	_("S&tore\tCtrl+Enter"),	_("Stores the symbol in the set"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_EXIT,							_("&Close\tAlt+F4"),		_("Closes the symbol editor"));
	menuBar->Append(menuFile, _("&File"));
	
	IconMenu* menuEdit = new IconMenu();
		menuEdit->Append(ID_EDIT_UNDO,		_("TOOL_UNDO"),		_("&Undo\tCtrl+Z"),			_("Undoes the last action"));
		menuEdit->Append(ID_EDIT_REDO,		_("TOOL_REDO"),		_("&Redo\tF4"),				_(""));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_DUPLICATE,	_("TOOL_DUPLICATE"),	_("&Duplicate\tCtrl+D"),_("Duplicates the selected shapes"));
	menuBar->Append(menuEdit, _("&Edit"));
	
	IconMenu* menuTool = new IconMenu();
		menuTool->Append(ID_MODE_SELECT,	_("TOOL_MODE_SELECT"),	_("&Select\tF5"),		_("Select and move shapes"), wxITEM_CHECK);
		menuTool->Append(ID_MODE_ROTATE,	_("TOOL_MODE_ROTATE"),	_("&Rotate\tF6"),		_("Rotate and shear shapes"), wxITEM_CHECK);
		menuTool->Append(ID_MODE_POINTS,	_("TOOL_MODE_CURVE"),	_("&Points\tF7"),		_("Edit control points for a shape in the symbol"), wxITEM_CHECK);
		menuTool->Append(ID_MODE_SHAPES,	_("TOOL_CIRCLE"),		_("&Basic Shapes\tF8"),	_("Draw basic shapes, such as rectangles and circles"), wxITEM_CHECK);
		menuTool->Append(ID_MODE_PAINT,		_("TOOL_MODE_PAINT"),	_("P&aint\tF9"),		_("Paint on the shape using a paintbrush"), wxITEM_CHECK);
	menuBar->Append(menuTool, _("&Tool"));
	
	SetMenuBar(menuBar);
	
	// Statusbar
	CreateStatusBar();
	SetStatusText(_(""));
	
	// Toolbar
	wxToolBar* tb = CreateToolBar(wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL | wxTB_TEXT);
	tb->AddTool(ID_FILE_STORE,	_("Store"),			Bitmap(_("TOOL_APPLY")),	wxNullBitmap, wxITEM_NORMAL, _("Store symbol in set"),	_("Stores the symbol in the set"));
	tb->AddSeparator();
	tb->AddTool(ID_EDIT_UNDO,	_("Undo"),			Bitmap(_("TOOL_UNDO")),		wxNullBitmap, wxITEM_NORMAL, _("Undo"),					_("Undoes the last action"));
	tb->AddTool(ID_EDIT_REDO,	_("Redo"),			Bitmap(_("TOOL_REDO")),		wxNullBitmap, wxITEM_NORMAL, _("Redo"),					_("Redoes the last action undone"));
	tb->Realize();
	
	// Edit mode toolbar
	wxPanel* emp = new wxPanel(this, wxID_ANY);
	wxToolBar* em = new wxToolBar(emp, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_VERTICAL | wxTB_TEXT | wxTB_HORZ_LAYOUT);
	em->AddTool(ID_MODE_SELECT,_("Select"),			Bitmap(_("TOOL_MODE_SELECT")),	wxNullBitmap, wxITEM_CHECK, _("Select (F5)"),			_("Select and move parts of the symbol"));
	em->AddTool(ID_MODE_ROTATE,_("Rotate"),			Bitmap(_("TOOL_MODE_ROTATE")),	wxNullBitmap, wxITEM_CHECK, _("Rotate (F6)"),			_("Rotate and shear parts of the symbol"));
	em->AddSeparator();
	em->AddTool(ID_MODE_POINTS,_("Points"),			Bitmap(_("TOOL_MODE_CURVE")),	wxNullBitmap, wxITEM_CHECK, _("Points (F7)"),			_("Edit control points for a shape in the symbol"));
	em->AddSeparator();
	em->AddTool(ID_MODE_SHAPES,_("Basic Shapes"),	Bitmap(_("TOOL_CIRCLE")),		wxNullBitmap, wxITEM_CHECK, _("Basic Shapes (F8)"),		_("Draw basic shapes, such as rectangles and circles"));
	em->AddSeparator();
	em->AddTool(ID_MODE_PAINT, _("Paint"),			Bitmap(_("TOOL_MODE_PAINT")),	wxNullBitmap, wxITEM_CHECK, _("Paint on shape (F9)"),	_("Paint on the shape using a paintbrush"));
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
	String name = wxFileSelector(_("Open symbol"),_(""),_(""),_(""),_("Symbol files|*.mse-symbol;*.bmp|MSE2 symbol files (*.mse-symbol)|*.mse-symbol|MSE1 symbol files (*.bmp)|*.bmp"),wxOPEN|wxFILE_MUST_EXIST, this);
	if (!name.empty()) {
		wxFileName n(name);
		String ext = n.GetExt();
		SymbolP symbol;
		if (ext.Lower() == _("bmp")) {
//%			symbol = importSymbol(wxImage(name));
		} else {
			Reader reader(new_shared1<wxFileInputStream>(name), name);
			reader.handle(symbol);
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
			ev.SetText(label + _("\tCtrl+Z"));
			GetToolBar()->SetToolShortHelp(ID_EDIT_UNDO, label);
			break;
		} case ID_EDIT_REDO: {
			ev.Enable(control->getSymbol()->actions.canRedo());
			String label = control->getSymbol()->actions.redoName();
			ev.SetText(label + _("\tF4"));
			GetToolBar()->SetToolShortHelp(ID_EDIT_REDO, label);
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
