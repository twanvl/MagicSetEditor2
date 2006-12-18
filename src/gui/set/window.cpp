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
#include <gui/set/keywords_panel.hpp>
#include <gui/set/stats_panel.hpp>
#include <gui/control/card_list.hpp>
#include <gui/control/gallery_list.hpp>
#include <gui/about_window.hpp>
#include <gui/update_checker.hpp>
#include <gui/new_window.hpp>
#include <gui/preferences_window.hpp>
#include <gui/print_window.hpp>
#include <gui/icon_menu.hpp>
#include <util/window_id.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <data/format/formats.hpp>

DECLARE_TYPEOF_COLLECTION(SetWindowPanel*);
DECLARE_TYPEOF_COLLECTION(SetWindow*);
DECLARE_TYPEOF_COLLECTION(String);

// ----------------------------------------------------------------------------- : Constructor

SetWindow::SetWindow(Window* parent, const SetP& set)
	: wxFrame(parent, wxID_ANY, _TITLE_("magic set editor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
	, current_panel(nullptr)
	, find_dialog(nullptr)
	, number_of_recent_sets(0)
{
	SetIcon(wxIcon(_("ICON_APP")));
	
	// initialize menu bar
	wxMenuBar* menuBar = new wxMenuBar();
	IconMenu* menuFile = new IconMenu();
		menuFile->Append(ID_FILE_NEW,		_("TOOL_NEW"),		_MENU_("new"),			_HELP_("new"));
		menuFile->Append(ID_FILE_OPEN,		_("TOOL_OPEN"),		_MENU_("open"),			_HELP_("open"));
		menuFile->Append(ID_FILE_SAVE,		_("TOOL_SAVE"),		_MENU_("save"),			_HELP_("save"));
		menuFile->Append(ID_FILE_SAVE_AS,						_MENU_("save as"),		_HELP_("save as"));
		IconMenu* menuExport = new IconMenu();
			menuExport->Append(ID_FILE_EXPORT_HTML,					_("&HTML..."),					_("Export the set to a HTML file"));
			menuExport->Append(ID_FILE_EXPORT_IMAGE,				_("Card &Image..."),			_("Export the selected card to an image file"));
			menuExport->Append(ID_FILE_EXPORT_IMAGES,				_("All Card I&mages..."),		_("Export images for all cards"));
			menuExport->Append(ID_FILE_EXPORT_APPR,					_("&Apprentice..."),			_("Export the set so it can be played with in Apprentice"));
			menuExport->Append(ID_FILE_EXPORT_MWS,					_("Magic &Workstation..."),		_("Export the set so it can be played with in Magic Workstation"));
		menuFile->Append(ID_FILE_EXPORT,						_MENU_("export"),					_("Export the set..."), menuExport);
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_INSPECT,						_("Inspect Internal Data..."),	_("Shows a the data in the set using a tree structure"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_PRINT_PREVIEW,					_MENU_("print preview"),	_HELP_("print preview"));
		menuFile->Append(ID_FILE_PRINT,							_MENU_("print"),			_HELP_("print"));
		menuFile->AppendSeparator();
		// recent files go here
		menuFile->AppendSeparator();
		menuFile->Append(ID_FILE_EXIT,							_MENU_("exit"),				_HELP_("exit"));
	menuBar->Append(menuFile, _MENU_("file"));
	
	IconMenu* menuEdit = new IconMenu();
		menuEdit->Append(ID_EDIT_UNDO,		_("TOOL_UNDO"),		_MENU_("undo"),				_HELP_("undo"));
		menuEdit->Append(ID_EDIT_REDO,		_("TOOL_REDO"),		_MENU_("redo"),				_HELP_("redo"));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_CUT,		_("TOOL_CUT"),		_MENU_("cut"),				_("Move the selected text to the clipboard"));
		menuEdit->Append(ID_EDIT_COPY,		_("TOOL_COPY"),		_MENU_("copy"),				_("Place the selected text on the clipboard"));
		menuEdit->Append(ID_EDIT_PASTE,		_("TOOL_PASTE"),	_MENU_("paste"),			_("Inserts the text from the clipboard"));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_FIND,		_("TOOL_FIND"),		_MENU_("find"),				_(""));
		menuEdit->Append(ID_EDIT_FIND_NEXT,						_MENU_("find next"),		_(""));
		menuEdit->Append(ID_EDIT_REPLACE,						_MENU_("replace"),			_(""));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_EDIT_PREFERENCES,					_MENU_("preferences"),		_("Change the configuration of Magic Set Editor"));
	menuBar->Append(menuEdit, _MENU_("edit"));
	
	IconMenu* menuWindow = new IconMenu();
		menuWindow->Append(ID_WINDOW_NEW,					 	_MENU_("new window"),		_HELP_("new window"));
		menuWindow->AppendSeparator();
	menuBar->Append(menuWindow, _MENU_("window"));
	
	IconMenu* menuHelp = new IconMenu();
		menuHelp->Append(ID_HELP_INDEX,		_("TOOL_HELP"),		_("&Index..\tF1"),				_(""));
		menuHelp->AppendSeparator();
		menuHelp->Append(ID_HELP_ABOUT,							_MENU_("about"),			_(""));
	menuBar->Append(menuHelp, _MENU_("help"));
	
	SetMenuBar(menuBar);
		
	// status bar
	CreateStatusBar();
	SetStatusText(_("Welcome to Magic Set Editor"));
			
	// tool bar
	wxToolBar* tb = CreateToolBar(wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL);
	tb->SetToolBitmapSize(wxSize(18,18));
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
	addPanel(menuWindow, tabBar, new CardsPanel   (this, wxID_ANY), 4, _("F5"), _("Cards"),		_("Cards"),				_("Edit the cards in the set"));
	addPanel(menuWindow, tabBar, new SetInfoPanel (this, wxID_ANY), 0, _("F6"), _("Set info"),	_("&Set Information"),	_("Edit information about the set, its creator, etc."));
	addPanel(menuWindow, tabBar, new StylePanel   (this, wxID_ANY), 1, _("F7"), _("Style"),		_("Style"),				_("Change the style of cards"));
	addPanel(menuWindow, tabBar, new KeywordsPanel(this, wxID_ANY), 2, _("F8"), _("Keywords"),	_("Keywords"),			_("Define extra keywords for this set"));
	addPanel(menuWindow, tabBar, new StatsPanel   (this, wxID_ANY), 3, _("F9"), _("Stats"),		_("Statistics"),		_("Show statistics about the cards in the set"));
//	addPanel(*s, *menuWindow, *tabBar, new DraftPanel   (&this, wxID_ANY), 4, _("F10")) 
	selectPanel(ID_WINDOW_MIN + 4); // select cards panel
	
	// loose ends
	tabBar->Realize();
	SetSize(settings.set_window_width, settings.set_window_height);
	if (settings.set_window_maximized) {
		Maximize();
	}
	set_windows.push_back(this); // register this window
	// don't send update ui events to children
	// note: this still sends events for menu and toolbar items!
	wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);
	SetExtraStyle(wxWS_EX_PROCESS_UI_UPDATES);
	
	setSet(set);
	current_panel->Layout();
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
	// remove from list of set windows
	set_windows.erase(remove(set_windows.begin(), set_windows.end(), this));
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

// ----------------------------------------------------------------------------- : Window managment

vector<SetWindow*> SetWindow::set_windows;

bool SetWindow::isOnlyWithSet() {
	FOR_EACH(w, set_windows) {
		if (w != this && w->set == set) return false;
	}
	return true;
}

// ----------------------------------------------------------------------------- : Set actions

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
	if (!isOnlyWithSet() || askSaveAndContinue()) {
		Destroy();
	} else {
		ev.Veto();
	}
}

bool SetWindow::askSaveAndContinue() {
	if (set->actions.atSavePoint()) return true;
	// todo : if more then one window has the set selected it's ok to proceed
	int save = wxMessageBox(_("The set has changed\n\nDo you want to save the changes?"), _("Save changes"), wxYES_NO | wxCANCEL | wxICON_EXCLAMATION);
	if (save == wxYES) {
		// save the set
		try {
			if (set->needSaveAs()) {
				// need save as
				wxFileDialog dlg(this, _("Save a set"), _(""), _(""), export_formats(*set->game), wxSAVE | wxOVERWRITE_PROMPT);
				if (dlg.ShowModal() == wxID_OK) {
					export_set(*set, dlg.GetPath(), dlg.GetFilterIndex());
					return true;
				} else {
					return false;
				}
			} else {
				set->save();
				set->actions.setSavePoint();
				return true;
			}
		} catch (Error e) {
			// something went wrong with saving, don't proceed
			handle_error(e);
			return false;
		}
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
		case ID_FILE_SAVE:         ev.Enable(!set->actions.atSavePoint() || set->needSaveAs());	break;
		case ID_FILE_EXPORT_IMAGE: ev.Enable(!!current_panel->selectedCard());					break;
		case ID_FILE_EXPORT_APPR:  ev.Enable(set->game->isMagic());								break;
		case ID_FILE_EXPORT_MWS:   ev.Enable(set->game->isMagic());								break;
		case ID_FILE_EXIT:
			// update for ID_FILE_RECENT done for a different id, because ID_FILE_RECENT may not be in the menu yet
			updateRecentSets();
			break;
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

static const int FILE_MENU_SIZE_BEFORE_RECENT_SETS = 11; // HACK; we should calculate the position to insert!
void SetWindow::updateRecentSets() {
	wxMenuBar* mb = GetMenuBar();
	assert(number_of_recent_sets <= (UInt)settings.recent_sets.size()); // the number of recent sets should only increase
	UInt i = 0;
	FOR_EACH(file, settings.recent_sets) {
		if (i >= settings.max_recent_sets) break;
		if (i < number_of_recent_sets) {
			// menu item already exists, update it
			mb->SetLabel(ID_FILE_RECENT + i, String(_("&")) << (i+1) << _(" ") << file);
		} else {
			// add new item
			int pos = i + FILE_MENU_SIZE_BEFORE_RECENT_SETS; // HUGE HACK, we should calculate the position to insert!
			IconMenu* file_menu = static_cast<IconMenu*>(mb->GetMenu(0));
			file_menu->Insert(pos, ID_FILE_RECENT + i, String(_("&")) << (i+1) << _(" ") << file, wxEmptyString);
		}
		i++;
	}
	number_of_recent_sets = (UInt)settings.recent_sets.size();
}

// ----------------------------------------------------------------------------- : Window events - menu - file


void SetWindow::onFileNew(wxCommandEvent&) {
	if (!askSaveAndContinue()) return;
	// new set?
	SetP new_set = new_set_window(this);
	if (new_set) setSet(new_set);
}

void SetWindow::onFileOpen(wxCommandEvent&) {
	if (!askSaveAndContinue())  return;
	wxFileDialog dlg(this, _("Open a set"), _(""), _(""), import_formats(), wxOPEN);
	if (dlg.ShowModal() == wxID_OK) {
		setSet(import_set(dlg.GetPath()));
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
	print_set(this, set);
}

void SetWindow::onFilePrintPreview(wxCommandEvent&) {
	print_preview(this, set);
}

void SetWindow::onFileRecent(wxCommandEvent& ev) {
	setSet(import_set(settings.recent_sets.at(ev.GetId() - ID_FILE_RECENT)));
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
	PreferencesWindow wnd(this);
	wnd.ShowModal();
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
	show_update_dialog(this);
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
	EVT_GALLERY_SELECT  (ID_FIELD_LIST,                SetWindow::onChildMenu) // for StatsPanel, because it is not a EVT_TOOL
	
	EVT_UPDATE_UI		(wxID_ANY,				SetWindow::onUpdateUI)
//	EVT_FIND			(wxID_ANY,				SetWindow::onFind)
//	EVT_FIND_NEXT		(wxID_ANY,				SetWindow::onFindNext)
//	EVT_FIND_REPLACE	(wxID_ANY,				SetWindow::onReplace)
//	EVT_FIND_REPLACE_ALL(wxID_ANY,				SetWindow::onReplaceAll)
	EVT_CLOSE			(						SetWindow::onClose)
	EVT_IDLE			(						SetWindow::onIdle)
	EVT_CARD_SELECT		(wxID_ANY,				SetWindow::onCardSelect)
END_EVENT_TABLE  ()
