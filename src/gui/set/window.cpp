//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/window.hpp>
#include <gui/set/panel.hpp>
#include <gui/set/cards_panel.hpp>
#include <gui/set/set_info_panel.hpp>
#include <gui/set/style_panel.hpp>
#include <gui/set/stats_panel.hpp>
#include <gui/control/card_list.hpp>
#include <gui/about_window.hpp>
#include <gui/new_window.hpp>
#include <gui/icon_menu.hpp>
#include <util/window_id.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <data/format/formats.hpp>

DECLARE_TYPEOF_COLLECTION(SetWindowPanel*);

// ----------------------------------------------------------------------------- : Constructor

SetWindow::SetWindow(Window* parent, const SetP& set)
	: wxFrame(parent, wxID_ANY, _("Magic Set Editor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
	, current_panel(nullptr)
	, find_dialog(nullptr)
{
	SetIcon(wxIcon(_("ICON_APP")));
	
	// initialize menu bar
	wxMenuBar* menuBar = new wxMenuBar();
	IconMenu* menuFile = new IconMenu();
		menuFile->Append(ID_FILE_NEW,		_("TOOL_NEW"),		_("&New...\tCtrl+N"),			_("Create a new set"));
		menuFile->Append(ID_FILE_OPEN,		_("TOOL_OPEN"),		_("&Open...\tCtrl+O"),			_("Open a set"));
		menuFile->Append(ID_FILE_SAVE,		_("TOOL_SAVE"),		_("&Save\tCtrl+S"),				_("Save the set"));
		menuFile->Append(ID_FILE_SAVE_AS,						_("Save &As...\tF12"),			_("Save the set with a new name"));
		IconMenu* menuExport = new IconMenu();
			menuExport->Append(ID_FILE_EXPORT_HTML,					_("&HTML..."),					_("Export the set to a HTML file"));
			menuExport->Append(ID_FILE_EXPORT_IMAGE,				_("Card &Image..."),			_("Export the selected card to an image file"));
			menuExport->Append(ID_FILE_EXPORT_IMAGES,				_("All Card I&mages..."),		_("Export images for all cards"));
			menuExport->Append(ID_FILE_EXPORT_APPR,					_("&Apprentice..."),			_("Export the set so it can be played with in Apprentice"));
			menuExport->Append(ID_FILE_EXPORT_MWS,					_("Magic &Workstation..."),		_("Export the set so it can be played with in Magic Workstation"));
		menuFile->Append(ID_FILE_EXPORT,						_("&Export"),					_("Export the set..."), menuExport);
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_INSPECT,						_("Inspect Internal Data..."),	_("Shows a the data in the set using a tree structure"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_PRINT_PREVIEW,					_("Print Pre&view..."),			_("Shows cards as they will be printed"));
		menuFile->Append(ID_FILE_PRINT,							_("&Print..."),					_("Print cards from this set"));
		menuFile->AppendSeparator();
		// recent files go here
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_EXIT,							_("E&xit\tAlt+F4"),				_("Quits Magic Set Editor; prompts to save the set"));
	menuBar->Append(menuFile, _("&File"));
	
	IconMenu* menuEdit = new IconMenu();
		menuEdit->Append(ID_EDIT_UNDO,		_("TOOL_UNDO"),		_("&Undo\tCtrl+Z"),				_("Undoes the last action"));
		menuEdit->Append(ID_EDIT_REDO,		_("TOOL_REDO"),		_("&Redo\tF4"),					_("Redoes the last action"));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_CUT,		_("TOOL_CUT"),		_("Cu&t\tCtrl+X"),				_("Move the selected text to the clipboard"));
		menuEdit->Append(ID_EDIT_COPY,		_("TOOL_COPY"),		_("&Copy\tCtrl+C"),				_("Place the selected text on the clipboard"));
		menuEdit->Append(ID_EDIT_PASTE,		_("TOOL_PASTE"),	_("&Paste\tCtrl+V"),			_("Inserts the text from the clipboard"));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_FIND,		_("TOOL_FIND"),		_("&Find...\tCtrl+F"),			_(""));
		menuEdit->Append(ID_EDIT_FIND_NEXT,						_("Find &Next\tF3"),			_(""));
		menuEdit->Append(ID_EDIT_REPLACE,						_("R&eplace...\tCtrl+H"),		_(""));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_PREFERENCES,					_("Preferences..."),			_("Change the configuration of Magic Set Editor"));
	menuBar->Append(menuEdit, _("&Edit"));
	
	IconMenu* menuWindow = new IconMenu();
		menuWindow->Append(ID_WINDOW_NEW,					_	("&New Window"),				_("Creates another window to edit the same set"));
		menuWindow->AppendSeparator();
	menuBar->Append(menuWindow, _("&Window"));
	
	IconMenu* menuHelp = new IconMenu();
		menuHelp->Append(ID_HELP_INDEX,		_("TOOL_HELP"),		_("&Index..\tF1"),				_(""));
		menuHelp->AppendSeparator();
		menuHelp->Append(ID_HELP_ABOUT,							_("&About Magic Set Editor..."), _(""));
	menuBar->Append(menuHelp, _("&Help"));
	
	SetMenuBar(menuBar);
		
	// status bar
	CreateStatusBar();
	SetStatusText(_("Welcome to Magic Set Editor"));
			
	// tool bar
	wxToolBar* tb = CreateToolBar(wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL);
	tb->AddTool(ID_FILE_NEW,	_(""),	Bitmap(_("TOOL_NEW")),	wxNullBitmap, wxITEM_NORMAL, _("New set"),	_("Creates a new set"));
	tb->AddTool(ID_FILE_OPEN,	_(""),	Bitmap(_("TOOL_OPEN")),	wxNullBitmap, wxITEM_NORMAL, _("Open set"),	_("Opens an existing set"));
	tb->AddTool(ID_FILE_SAVE,	_(""),	Bitmap(_("TOOL_SAVE")),	wxNullBitmap, wxITEM_NORMAL, _("Save set"),	_("Saves the current set"));
	tb->AddSeparator();
	tb->AddTool(ID_EDIT_CUT,	_(""),	Bitmap(_("TOOL_CUT")),	wxNullBitmap, wxITEM_NORMAL, _("Cut"));
	tb->AddTool(ID_EDIT_COPY,	_(""),	Bitmap(_("TOOL_COPY")),	wxNullBitmap, wxITEM_NORMAL, _("Copy"));
	tb->AddTool(ID_EDIT_PASTE,	_(""),	Bitmap(_("TOOL_PASTE")),wxNullBitmap, wxITEM_NORMAL, _("Paste"));
	tb->AddSeparator();
	tb->AddTool(ID_EDIT_UNDO,	_(""),	Bitmap(_("TOOL_UNDO")),	wxNullBitmap, wxITEM_NORMAL, _("Undo"));
	tb->AddTool(ID_EDIT_REDO,	_(""),	Bitmap(_("TOOL_REDO")),	wxNullBitmap, wxITEM_NORMAL, _("Redo"));
	tb->AddSeparator();
	tb->Realize();
	
	// tab bar, sizer
	wxToolBar* tabBar = new wxToolBar(this, ID_TAB_BAR, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL | wxTB_HORZ_TEXT | wxTB_NOICONS);
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(tabBar, 0, wxEXPAND | wxBOTTOM, 3);
	SetSizer(s);
	
	// panels
	// NOTE: place the CardsPanel last in the panels list,
	//  this way the card list is the last to be told of a set change
	//  this way everyone else already uses the new set when it sends a CardSelectEvent
	addPanel(menuWindow, tabBar, new CardsPanel   (this, wxID_ANY), 3, _("F5"), _("Cards"),		_("Cards"),				_("Edit the cards in the set"));
	addPanel(menuWindow, tabBar, new SetInfoPanel (this, wxID_ANY), 0, _("F6"), _("Set info"),	_("&Set Information"),	_("Edit information about the set, its creator, etc."));
	addPanel(menuWindow, tabBar, new StylePanel   (this, wxID_ANY), 1, _("F7"), _("Style"),		_("Style"),				_("Change the style of cards"));
//	addPanel(menuWindow, tabBar, new KeywordsPanel(this, wxID_ANY), 2, _("F8"));
	addPanel(menuWindow, tabBar, new StatsPanel   (this, wxID_ANY), 2, _("F9"), _("Stats"),		_("Statistics"),		_("Show statistics about the cards in the set"));
//	addPanel(*s, *menuWindow, *tabBar, new DraftPanel   (&this, wxID_ANY), 4, _("F10")) 
	selectPanel(ID_WINDOW_MIN + 3); // select cards panel
	
	// loose ends
	tabBar->Realize();
	SetSize(settings.set_window_width, settings.set_window_height);
	if (settings.set_window_maximized) {
		Maximize();
	}
//	SetWindows.push_back(&this); // register this window
//	timer.owner = &this;
//	timer.start(10);
	// don't send update ui events to children
	// note: this still sends events for menu and toolbar items!
	wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);
	SetExtraStyle(wxWS_EX_PROCESS_UI_UPDATES);
	
	setSet(set);
}

SetWindow::~SetWindow() {
	// store window size in settings
	wxSize s = GetSize();
	settings.set_window_maximized = IsMaximized();
	if (!IsMaximized()) {
		settings.set_window_width  = s.GetWidth();
		settings.set_window_height = s.GetHeight();
	}
	// destroy ui of selected panel
	current_panel->destroyUI(GetToolBar(), GetMenuBar());
	// cleanup (see find stuff)
	delete find_dialog;
	// remove from list of main windows
//	SetWindows.erase(remove(SetWindows.begin(), SetWindows.end(), &this));
	// stop updating
	onBeforeChangeSet();
}


// ----------------------------------------------------------------------------- : Panel managment

void SetWindow::addPanel(wxMenu* windowMenu, wxToolBar* tabBar, SetWindowPanel* panel, UInt pos,
                         const String& shortcut, const String& shortName, const String& longName, const String& description) {
	// insert in list
	if (panels.size() <= pos) panels.resize(pos + 1);
	panels[pos] = panel;
	// add to tab bar
	int id = ID_WINDOW_MIN + pos;
	tabBar->AddTool(id, shortName, wxNullBitmap, wxNullBitmap, wxITEM_CHECK, longName, description);
	// add to menu bar
	windowMenu->AppendCheckItem(id, longName + _("\t") + shortcut, description);
	// add to sizer
	GetSizer()->Add(panel, 1, wxEXPAND);
}

void SetWindow::selectPanel(int id) {
	SetWindowPanel* toSelect = panels.at(id - ID_WINDOW_MIN);
	if (current_panel != toSelect) {
		// destroy & create menus
		if (current_panel) current_panel->destroyUI(GetToolBar(), GetMenuBar());
		current_panel = toSelect;
		current_panel->initUI(GetToolBar(), GetMenuBar());
	}
	// show/hide panels and select tabs
	wxSizer*   sizer   = GetSizer();
	wxToolBar* tabBar  = (wxToolBar*)FindWindow(ID_TAB_BAR);
	wxMenuBar* menuBar = GetMenuBar();
	int wid = ID_WINDOW_MIN;
	FOR_EACH(p, panels) {
		sizer->Show       (p,   p == current_panel);
		tabBar->ToggleTool(wid, p == current_panel);
		menuBar->Check    (wid, p == current_panel);
		++wid;
	}
	// fix sizer stuff
	fixMinWindowSize();
}

// ----------------------------------------------------------------------------- : Set actions

void SetWindow::onBeforeChangeSet() {
	// Make sure we don't own the set's scriptUpdater
//	if (set->scriptUpdater != &scriptUpdater) {
//		assert(!scriptUpdater.set);
//		return;
//	}
	// We do own the set's scriptUpdater
	// stop handling updates for the set
	// do this first, so events get send, which stop worker threads
//	scriptUpdater.setNullSet();
	// another window should take over the scriptUpdating
//	FOR_EACH(wnd, SetWindows) {
//		if (wnd != &this && wnd->getSet() == set) {
//			wnd->scriptUpdater.setSet(set);
//			break;
//		}
//	}
}

void SetWindow::onChangeSet() {
	// make sure there is always at least one card
	// some things need this
	if (set->cards.empty()) set->cards.push_back(new_shared1<Card>(*set->game));
	// all panels view the same set
	FOR_EACH(p, panels) {
		p->setSet(set);
	}
	fixMinWindowSize();
}

void SetWindow::onAction(const Action& action, bool undone) {
//	TYPE_CASE_(action, SetStyleChange) {
//		// The style changed, maybe also the size of the viewer
//		Layout();
//		fixMinWindowSize();
//	}
}
	
	
void SetWindow::onCardSelect(CardSelectEvent& ev) {
	FOR_EACH(p, panels) {
		p->selectCard(ev.card);
	}
	fixMinWindowSize();
}

void SetWindow::onRenderSettingsChange() {
	FOR_EACH(p, panels) {
		p->onRenderSettingsChange();
	}
	fixMinWindowSize();
}

void SetWindow::fixMinWindowSize() {
	current_panel->Layout();
	current_panel->SetMinSize(current_panel->GetSizer()->GetMinSize());
	Layout();
	wxSize s  = GetSizer()->GetMinSize();
	wxSize ws = GetSize();
	wxSize cs = GetClientSize();
	wxSize minSize = wxSize(s.x + ws.x - cs.x, s.y + ws.y - cs.y);
	if (ws.x < minSize.x) ws.x = minSize.x;
	if (ws.y < minSize.y) ws.y = minSize.y;
	SetSize(ws);
	SetMinSize(minSize);
}



// ----------------------------------------------------------------------------- : Window events - close

void SetWindow::onClose(wxCloseEvent& ev) {
	// only ask if we want to save is this is the only window that has the current set opened
//	if (!isOnlyWithSet() || askSaveAndContinue()) {
//		timer.stop();
		Destroy();
//	} else {
//		ev.Veto();
//	}
}

bool SetWindow::askSaveAndContinue() {
	if (set->actions.atSavePoint()) return true;
	// todo : if more then one window has the set selected it's ok to proceed
	int save = wxMessageBox(_("The set has changed\n\nDo you want to save the changes?"), _("Save changes"), wxYES_NO | wxCANCEL | wxICON_EXCLAMATION);
	if (save == wxYES) {
		// save the set
/*		try {
			if (set->needSaveAs()) {
				// need save as
				FileDialog dlg(&this, _("Save a set"), _(""), _(""), export_formats(set->game), wxSAVE | wxOVERWRITE_PROMPT);
				if (dlg.showModal() == wxID_OK) {
					exportSet(set, dlg.path, dlg.filterIndex);
					return true;
				} else {
					return false;
				}
			} else {
				set->save();
				set->actions.atSavePoint = true;
				return true;
			}
		} catch (Error e) {
			// something went wrong with saving, don't proceed
			handleError(e);
			return false;
		}
*/		return false;/////<<<<removeme
	} else if (save == wxNO) {
		return true;
	} else { // wxCANCEL
		return false;
	}
}

// ----------------------------------------------------------------------------- : Window events - update UI

void SetWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		// file menu
		case ID_FILE_SAVE_AS:      ev.Enable(!set->actions.atSavePoint() || set->needSaveAs());	break;
		case ID_FILE_EXPORT_IMAGE: ev.Enable(!!current_panel->selectedCard());					break;
		case ID_FILE_EXPORT_APPR:  ev.Enable(set->game->isMagic());								break;
		case ID_FILE_EXPORT_MWS:   ev.Enable(set->game->isMagic());								break;
		/*case ID_FILE_INSPECT: {
			// the item just before FileRecent, because FileRecent may not be in the menu yet
			//updateRecentSets();
			// TODO
			break;
		}*/
		// undo/redo
		case ID_EDIT_UNDO: {
			ev.Enable(set->actions.canUndo());
			String label = set->actions.undoName();
			ev.SetText(label + _("\tCtrl+Z"));
			GetToolBar()->SetToolShortHelp(ID_EDIT_UNDO, label);
			break;
		} case ID_EDIT_REDO: {
			ev.Enable(set->actions.canRedo());
			String label = set->actions.redoName();
			ev.SetText(label + _("\tF4"));
			GetToolBar()->SetToolShortHelp(ID_EDIT_REDO, label);
			break;
		}
		// copy & paste & find
		case ID_EDIT_CUT       : ev.Enable(current_panel->canCut());	break;
		case ID_EDIT_COPY      : ev.Enable(current_panel->canCopy());	break;
		case ID_EDIT_PASTE     : ev.Enable(current_panel->canPaste());	break;
		case ID_EDIT_FIND      : ev.Enable(current_panel->canFind());	break;
		case ID_EDIT_FIND_NEXT : ev.Enable(current_panel->canFind());	break;
		case ID_EDIT_REPLACE   : ev.Enable(current_panel->canReplace());break;
		default:
			// items created by the panel, and cut/copy/paste and find/replace
			current_panel->onUpdateUI(ev);
	}
}

// ----------------------------------------------------------------------------- : Window events - menu - file


void SetWindow::onFileNew(wxCommandEvent&) {
	if (!askSaveAndContinue()) return;
	// new set?
	SetP new_set = new_set_window(this);
	if (new_set) set = new_set;
}

void SetWindow::onFileOpen(wxCommandEvent&) {
	if (!askSaveAndContinue())  return;
	wxFileDialog dlg(this, _("Open a set"), _(""), _(""), import_formats(), wxOPEN);
	if (dlg.ShowModal() == wxID_OK) {
		set = import_set(dlg.GetPath());
	}
}

void SetWindow::onFileSave(wxCommandEvent& ev) {
	if (set->needSaveAs()) {
		onFileSaveAs(ev);
	} else {
		settings.addRecentFile(set->absoluteFilename());
		set->save();
		set->actions.setSavePoint();
	}
}

void SetWindow::onFileSaveAs(wxCommandEvent&) {
	wxFileDialog dlg(this, _("Save a set"), _(""), _(""), export_formats(*set->game), wxSAVE | wxOVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) {
		export_set(*set, dlg.GetPath(), dlg.GetFilterIndex());
	}
}

/*
void SetWindow::onFileInspect(wxCommandEvent&) {
	var wnd = new TreeGridWindow(&this, set);
	wnd.show();
}*/

void SetWindow::onFileExportImage(wxCommandEvent&) {
	CardP card = current_panel->selectedCard();
	if (!card)  return; // no card selected
	String name = wxFileSelector(_("Save image"), _(""), card->identification(), _(""),
		                         _("JPEG images (*.jpg)|*.jpg|Windows bitmaps (*.bmp)|*.bmp|PNG images (*.png)|*.png|GIF images (*.gif)|*.gif|TIFF images (*.tif)|*.tif"),
		                         wxSAVE | wxOVERWRITE_PROMPT, this);
	if (!name.empty()) {
//		exportImage(set, card, name);
	}
}

void SetWindow::onFileExportImages(wxCommandEvent&) {
//	exportImages(this, set);
}

void SetWindow::onFileExportHTML(wxCommandEvent&) {
//	HtmlExportWindow wnd(&this, set);
//	wnd.ShowModal();
	/*;//%%
	String name = fileSelector(_("Exort to html"),_(""),_(""),_(""), {
		                        _("HTML files (*.html)|*.html"),
		                        wxSAVE | wxOVERWRITE_PROMPT);
	}
	if (!name.empty()) {
		HtmlExportWindow wnd(&this, set, name);
		wnd.showModal();
	}
	*/
}

void SetWindow::onFileExportApprentice(wxCommandEvent&) {
//	ApprenticeExportWindow wnd(&this, set);
//	wnd.ShowModal();
}

void SetWindow::onFileExportMWS(wxCommandEvent&) {
//	exportMWS(set);
}

void SetWindow::onFilePrint(wxCommandEvent&) {
//	printSet(this, set);
}

void SetWindow::onFilePrintPreview(wxCommandEvent&) {
//	printPreview(this, set);
}

void SetWindow::onFileRecent(wxCommandEvent& ev) {
//	setSet(import_set(settings.recentSets.at(ev.GetId() - ID_FILE_RECENT)));
}

void SetWindow::onFileExit(wxCommandEvent&) {
	Close();
}

// ----------------------------------------------------------------------------- : Window events - menu - edit

void SetWindow::onEditUndo(wxCommandEvent&) {
	set->actions.undo();
}
void SetWindow::onEditRedo(wxCommandEvent&) {
	set->actions.redo();
}

void SetWindow::onEditCut  (wxCommandEvent&) {
	current_panel->doCut();
}
void SetWindow::onEditCopy (wxCommandEvent&) {
	current_panel->doCopy();
}
void SetWindow::onEditPaste(wxCommandEvent&) {
	current_panel->doPaste();
}

void SetWindow::onEditFind(wxCommandEvent&) {
	delete find_dialog;
	find_dialog = new wxFindReplaceDialog(this, &find_data, _("Find"));
	find_dialog->Show();
}
void SetWindow::onEditFindNext(wxCommandEvent&) {
	current_panel->doFind(find_data);
}
void SetWindow::onEditReplace(wxCommandEvent&) {
	delete find_dialog;
	find_dialog = new wxFindReplaceDialog(this, &find_data, _("Replace"), wxFR_REPLACEDIALOG);
	find_dialog->Show();
}

// find events
void SetWindow::onFind      (wxFindDialogEvent&) {
	current_panel->doFind(find_data);
}
void SetWindow::onFindNext  (wxFindDialogEvent&) {
	current_panel->doFind(find_data);
}
void SetWindow::onReplace   (wxFindDialogEvent&) {
	current_panel->doReplace(find_data);
}
void SetWindow::onReplaceAll(wxFindDialogEvent&) {
	// todo
}

void SetWindow::onEditPreferences(wxCommandEvent&) {
//	SettingsWindow wnd(this);
//	if (wnd.ShowModal() == wxID_OK) {
//		// render settings may have changed, notify all windows
//		FOR_EACH(m, setWindows) {
//			m->onRenderSettingsChange();
//		}
//	}
}


// ----------------------------------------------------------------------------- : Window events - menu - window

void SetWindow::onWindowNewWindow(wxCommandEvent&) {
	SetWindow* newWindow = new SetWindow(nullptr, set);
	newWindow->Show();
}

void SetWindow::onWindowSelect(wxCommandEvent& ev) {
	selectPanel(ev.GetId());
}


// ----------------------------------------------------------------------------- : Window events - menu - help

void SetWindow::onHelpIndex(wxCommandEvent&) {
	// TODO : context sensitive
//	HelpWindow* wnd = new HelpWindow(this);
//	wnd->Show();
}

void SetWindow::onHelpAbout(wxCommandEvent&) {
	AboutWindow wnd(this);
	wnd.ShowModal();
}

// ----------------------------------------------------------------------------- : Window events - other

void SetWindow::onChildMenu(wxCommandEvent& ev) {
	current_panel->onCommand(ev.GetId());
}

void SetWindow::onIdle(wxIdleEvent& ev) {
	// Stuff that must be done in the main thread
	handle_pending_errors();
//	showUpdateDialog(this);
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(SetWindow, wxFrame)
	EVT_MENU			(ID_FILE_NEW,			SetWindow::onFileNew)
	EVT_MENU			(ID_FILE_OPEN,			SetWindow::onFileOpen)
	EVT_MENU			(ID_FILE_SAVE,			SetWindow::onFileSave)
	EVT_MENU			(ID_FILE_SAVE_AS,		SetWindow::onFileSaveAs)
	EVT_MENU			(ID_FILE_EXPORT_IMAGE,	SetWindow::onFileExportImage)
	EVT_MENU			(ID_FILE_EXPORT_IMAGES,	SetWindow::onFileExportImages)
	EVT_MENU			(ID_FILE_EXPORT_HTML,	SetWindow::onFileExportHTML)
	EVT_MENU			(ID_FILE_EXPORT_APPR,	SetWindow::onFileExportApprentice)
	EVT_MENU			(ID_FILE_EXPORT_MWS,	SetWindow::onFileExportMWS)
//	EVT_MENU			(ID_FILE_INSPECT,		SetWindow::onFileInspect)
	EVT_MENU			(ID_FILE_PRINT,			SetWindow::onFilePrint)
	EVT_MENU			(ID_FILE_PRINT_PREVIEW,	SetWindow::onFilePrintPreview)
	EVT_MENU_RANGE		(ID_FILE_RECENT, ID_FILE_RECENT_MAX, SetWindow::onFileRecent)
	EVT_MENU			(ID_FILE_EXIT,			SetWindow::onFileExit)
	EVT_MENU			(ID_EDIT_UNDO,			SetWindow::onEditUndo)
	EVT_MENU			(ID_EDIT_REDO,			SetWindow::onEditRedo)
	EVT_MENU			(ID_EDIT_CUT,			SetWindow::onEditCut)
	EVT_MENU			(ID_EDIT_COPY,			SetWindow::onEditCopy)
	EVT_MENU			(ID_EDIT_PASTE,			SetWindow::onEditPaste)
	EVT_MENU			(ID_EDIT_FIND,			SetWindow::onEditFind)
	EVT_MENU			(ID_EDIT_FIND_NEXT,		SetWindow::onEditFindNext)
	EVT_MENU			(ID_EDIT_REPLACE,		SetWindow::onEditReplace)
	EVT_MENU			(ID_EDIT_PREFERENCES,	SetWindow::onEditPreferences)
	EVT_MENU			(ID_WINDOW_NEW,			SetWindow::onWindowNewWindow)
	EVT_TOOL_RANGE		(ID_WINDOW_MIN, ID_WINDOW_MAX, SetWindow::onWindowSelect)
	EVT_MENU			(ID_HELP_INDEX,			SetWindow::onHelpIndex)
	EVT_MENU			(ID_HELP_ABOUT,			SetWindow::onHelpAbout)
	EVT_TOOL_RANGE		(ID_CHILD_MIN, ID_CHILD_MAX,   SetWindow::onChildMenu)
	
	EVT_UPDATE_UI		(wxID_ANY,				SetWindow::onUpdateUI)
//	EVT_FIND			(wxID_ANY,				SetWindow::onFind)
//	EVT_FIND_NEXT		(wxID_ANY,				SetWindow::onFindNext)
//	EVT_FIND_REPLACE	(wxID_ANY,				SetWindow::onReplace)
//	EVT_FIND_REPLACE_ALL(wxID_ANY,				SetWindow::onReplaceAll)
	EVT_CLOSE			(						SetWindow::onClose)
	EVT_IDLE			(						SetWindow::onIdle)
	EVT_CARD_SELECT		(wxID_ANY,				SetWindow::onCardSelect)
END_EVENT_TABLE  ()
