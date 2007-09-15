//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/update_checker.hpp>
#include <data/settings.hpp>
#include <util/io/package_manager.hpp>
#include <util/version.hpp>
#include <util/window_id.hpp>
#include <script/value.hpp> // for some strange reason the profile build needs this :(
#include <script/to_value.hpp>
#include <wx/dialup.h>
#include <wx/url.h>
#include <wx/html/htmlwin.h>
#include <wx/vlbox.h>

DECLARE_POINTER_TYPE(PackageVersionData);
DECLARE_POINTER_TYPE(VersionData);

DECLARE_TYPEOF_COLLECTION(PackageVersionDataP);

// ----------------------------------------------------------------------------- : Update data

/// Information on available packages
class PackageVersionData : public IntrusivePtrBase<PackageVersionData> {
  public:
	PackageVersionData() : is_installer(true) {}
	
	String  name;						///< Name of the package
	String  description;				///< html description
	String  url;						///< Where can the package be downloaded?
	bool    is_installer;				///< Download url refers to a .mse-installer
	Version version;					///< Version number of the download
	Version app_version;				///< The minimium version of MSE required
	vector<PackageDependencyP> depends;	///< Packages this depends on

	DECLARE_REFLECTION();
};

/// Information on the latest availible version
class VersionData : public IntrusivePtrBase<VersionData> {
  public:
	Version version;				///< Latest version number of MSE
	String  description;			///< html description of the latest MSE release
	String  new_updates_url;		///< updates url has moved?
	vector<PackageVersionDataP> packages;	///< Available packages
	
	DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(PackageVersionData) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(url);
	REFLECT(is_installer);
	REFLECT(version);
	REFLECT(app_version);
	REFLECT_N("depends ons", depends);
}

IMPLEMENT_REFLECTION(VersionData) {
	REFLECT(version);
	REFLECT(description);
	REFLECT(new_updates_url);
	REFLECT(packages);
}

// The information for the latest version
VersionDataP update_version_data;
// Have we shown the update dialog?
bool shown_dialog = false;
// Is update checking in progress?
volatile bool checking_updates = false;

bool update_data_found() { return !!update_version_data; }
bool update_available()  { return update_version_data && update_version_data->version > app_version; }

// ----------------------------------------------------------------------------- : Update checking

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(UPDATE_CHECK_FINISHED_EVT, -1)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(UPDATE_CHECK_FINISHED_EVT)

// Thread to retrieve update information
// Checks if the current version is the latest version
// If not, displays a message
class CheckUpdateThread : public wxThread {
  public:
	virtual void* Entry() {
		Work();
		return 0;;
	}
	
	static void Work() {
		if (checking_updates) return; // don't check multiple times simultaniously
		checking_updates = true;
		try {
			wxURL url(settings.updates_url);
			wxInputStream* isP = url.GetInputStream();
			if (!isP) return; // failed to get data
			InputStreamP is(isP);
			// Read version data
			VersionDataP version_data;
			Reader reader(is, _("updates"));
			reader.handle(version_data);
			// has the updates url changed?
			if (!version_data->new_updates_url.empty()) {
				settings.updates_url = version_data->new_updates_url;
			}
			// Make available
			update_version_data = version_data;
		} catch (...) {
			// ignore all errors, we don't want problems if update checking fails
		}
		checking_updates = false;
	}
};

void check_updates() {
	if (settings.check_updates == CHECK_ALWAYS) {
		check_updates_now();
	} else if (settings.check_updates == CHECK_IF_CONNECTED) {
		// only if internet connection exists
		wxDialUpManager* dum = wxDialUpManager::Create();
		if (dum->IsOk() && dum->IsOnline()) {
			check_updates_now();
		}
		delete dum;
	}
}

void check_updates_now(bool async) {
	wxSocketBase::Initialize();
	if (async) {
		CheckUpdateThread* thread = new CheckUpdateThread;
		thread->Create();
		thread->Run();
	} else {
		CheckUpdateThread::Work();
	}
}


// ----------------------------------------------------------------------------- : Dialog

// A HTML control that opens all pages in an actual browser
struct HtmlWindowToBrowser : public wxHtmlWindow {
	HtmlWindowToBrowser(Window* parent, int id, const wxPoint& pos, const wxSize& size, long flags)
		: wxHtmlWindow(parent, id, pos, size, flags)
	{}
		
	virtual void OnLinkClicked(const wxHtmlLinkInfo& info) {
		wxLaunchDefaultBrowser( info.GetHref() );
	}
};

void show_update_dialog(Window* parent) {
	if (!update_available() || shown_dialog) return; // we already have the latest version, or this has already been displayed.
	// Show update dialog
	wxDialog* dlg = new wxDialog(parent, wxID_ANY, _TITLE_("updates availible"), wxDefaultPosition);
	// controls
	wxHtmlWindow* html = new HtmlWindowToBrowser(dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER);
	html->SetPage(update_version_data->description);
	wxButton* close = new wxButton(dlg, wxID_OK, _BUTTON_("close"));
	close->SetDefault();
	// layout
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(html, 1, wxEXPAND | wxALL, 8);
	s->Add(close, 0, wxALIGN_RIGHT | wxALL & ~wxTOP, 8);
	dlg->SetSizer(s);
	dlg->SetSize(400,400);
	dlg->Show();
	// And never show it again this run
	shown_dialog = true;
}

// ----------------------------------------------------------------------------- : PackageUpdateList

class PackageUpdateList : public wxVListBox {
  public:
	PackageUpdateList(UpdatesWindow* parent)
		: wxVListBox (parent, ID_PACKAGE_LIST, wxDefaultPosition, wxSize(480,210), wxNO_BORDER | wxVSCROLL)
		, parent(parent)
	{
		if (!checking_updates && !update_version_data) {
			check_updates_now(true);
		}
		SetItemCount(update_version_data ? update_version_data->packages.size() : 1);
	}
	
	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
		static wxBrush greyBrush(Color(224,224,224));
		if (checking_updates) {
			String text = _ERROR_("checking updates");
			
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(greyBrush);
			dc.DrawRectangle(rect);
			
			int w, h;
			dc.GetTextExtent(text, &w, &h);
			dc.DrawText(text, rect.GetLeft() + (rect.GetWidth() - w) / 2, rect.GetTop() + (rect.GetHeight() - h) / 2);
		} else if (!update_version_data || update_version_data->packages.empty()) {
			String text = _ERROR_("no packages");
			
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(greyBrush);
			dc.DrawRectangle(rect);
			
			int w, h;
			dc.GetTextExtent(text, &w, &h);
			dc.DrawText(text, rect.GetLeft() + (rect.GetWidth() - w) / 2, rect.GetTop() + (rect.GetHeight() - h) / 2);
		} else {
			static wxBrush darkBrush(Color(192,224,255));
			static wxBrush selectBrush(Color(96,96,192));
			
			PackageVersionDataP pack = update_version_data->packages[n];
			UpdatesWindow::PackageStatus status = parent->package_data[pack].first;
			UpdatesWindow::PackageAction action = parent->package_data[pack].second;
			
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(IsSelected(n) ? selectBrush : (n % 2 ? *wxWHITE_BRUSH : darkBrush));
			
			dc.DrawRectangle(rect);
			
			// These two arrays correspond to PackageStatus and PackageAction, respectively
			static Color status_colors [] = {
				 Color(32,160,32)
				,Color(32,32,255)
				,Color(192,32,32)
			};
			
			static Color action_colors [] = {
				 Color(32,160,32)
				,Color(192,32,32)
				,Color(192,192,32)
				,Color(32,32,255)
				,Color(32,32,32)
			};
			
			// Ditto here (these are the locale names)
			static String status_texts [] = {
				 _TYPE_("installed")
				,_TYPE_("uninstalled")
				,_TYPE_("upgradeable")
			};
			
			static String action_texts [] = {
				 _TYPE_("install")
				,_TYPE_("uninstall")
				,_TYPE_("upgrade")
				,_TYPE_("do nothing")
				,_TYPE_("new mse")
			};
			
			static Color packageFront(64,64,64);
			
			#define SELECT_WHITE(color) (IsSelected(n) ? *wxWHITE : color)
			
			dc.SetTextForeground(SELECT_WHITE(packageFront));
			dc.DrawText(pack->name, rect.GetLeft() + 1, rect.GetTop());
			
			dc.SetTextForeground(SELECT_WHITE(status_colors[status]));
			dc.DrawText(status_texts[status], rect.GetLeft() + 240, rect.GetTop());
			
			dc.SetTextForeground(SELECT_WHITE(action_colors[action]));
			dc.DrawText(action_texts[action], rect.GetLeft() + 360, rect.GetTop());
			
			#undef SELECT_WHITE
		}
	}
	
	virtual wxCoord OnMeasureItem(size_t) const {
		return (update_version_data && !update_version_data->packages.empty()) ? 15 : 210;
	}
	
	void onUpdateCheckingFinished(wxEvent& event) {
		SetItemCount(update_version_data ? update_version_data->packages.size() : 1);
	}
	
	virtual wxCoord EstimateTotalHeight() const {
		return (update_version_data && !update_version_data->packages.empty())
		           ? 15 * (int)update_version_data->packages.size()
		           : 210;
	}
  private:
	DECLARE_EVENT_TABLE()

	UpdatesWindow* parent;
};

BEGIN_EVENT_TABLE(PackageUpdateList, wxVListBox)
	EVT_CUSTOM(UPDATE_CHECK_FINISHED_EVT, wxID_ANY, PackageUpdateList::onUpdateCheckingFinished)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : UpdateWindow

UpdatesWindow::UpdatesWindow()
	: Frame(nullptr, wxID_ANY, _TITLE_("package list"), wxDefaultPosition, wxSize(480,440), wxDEFAULT_DIALOG_STYLE | wxCLIP_CHILDREN)
{
	SetIcon(wxIcon());
	wxBoxSizer *v = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *h1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *h2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *h3 = new wxBoxSizer(wxHORIZONTAL);
	
	package_list = new PackageUpdateList(this);
	description_window = new HtmlWindowToBrowser(this, wxID_ANY, wxDefaultPosition, wxSize(480,100), wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER);
	
	setDefaultPackageStatus();
	
	package_title = new wxStaticText(this, wxID_ANY, _TITLE_("package name"),   wxDefaultPosition, wxSize(120,15), wxALIGN_LEFT | wxST_NO_AUTORESIZE);
	status_title  = new wxStaticText(this, wxID_ANY, _TITLE_("package status"), wxDefaultPosition, wxSize(120,15), wxALIGN_LEFT | wxST_NO_AUTORESIZE);
	new_title     = new wxStaticText(this, wxID_ANY, _TITLE_("new status"),     wxDefaultPosition, wxSize(120,15), wxALIGN_LEFT | wxST_NO_AUTORESIZE);
	
	h1->Add(package_title);
	h1->Add(status_title);
	h1->Add(new_title, 2);
	
	(install_button = new wxButton(this, ID_INSTALL, _MENU_("install package")))->Disable();
	(upgrade_button = new wxButton(this, ID_UPGRADE, _MENU_("upgrade package")))->Disable();
	(remove_button  = new wxButton(this, ID_REMOVE,  _MENU_("remove package")))->Disable();
	(cancel_button  = new wxButton(this, ID_CANCEL,  _MENU_("cancel changes")))->Disable();
	
	h2->AddStretchSpacer(1);
	h2->Add(install_button);
	h2->AddStretchSpacer(2);
	h2->Add(upgrade_button);
	h2->AddStretchSpacer(2);
	h2->Add(remove_button);
	h2->AddStretchSpacer(2);
	h2->Add(cancel_button);
	h2->AddStretchSpacer(1);

	apply_button = new wxButton(this, ID_APPLY,   _MENU_("apply changes"));

	h3->AddStretchSpacer(1);
	h3->Add(apply_button);
	h3->AddStretchSpacer(1);
	
	v->Add(h1);
	v->Add(package_list);
	v->AddStretchSpacer(2);
	v->Add(description_window);
	v->AddStretchSpacer(2);
	v->Add(h2);
	v->AddStretchSpacer(1);
	v->Add(h3);
	v->AddStretchSpacer(2);
	
	SetSizer(v);
}

void UpdatesWindow::onUpdateCheckFinished(wxCommandEvent&) {
	setDefaultPackageStatus();
}

void UpdatesWindow::onPackageSelect(wxCommandEvent& ev) {
	updateButtons(ev.GetInt());
}

void UpdatesWindow::onActionChange(wxCommandEvent& ev) {
	PackageVersionDataP pack = update_version_data->packages[package_list->GetSelection()];
	PackageAction& action = package_data[pack].second;
	switch (ev.GetId()) {
		case ID_INSTALL: action = ACTION_INSTALL; break;
		case ID_REMOVE:  action = ACTION_UNINSTALL; break;
		case ID_UPGRADE: action = ACTION_UPGRADE; break;
		case ID_CANCEL:  action = (pack->app_version > file_version) ? ACTION_NEW_MSE : ACTION_NOTHING; break;
	}
	updateButtons(package_list->GetSelection());
	package_list->Refresh();
}

void UpdatesWindow::onApplyChanges(wxCommandEvent& ev) {
}

void UpdatesWindow::updateButtons(int id) {
	PackageVersionDataP pack = update_version_data->packages[id];

	description_window->SetPage(pack->description);

	PackageStatus status = package_data[pack].first;
	PackageAction action = package_data[pack].second;

	if (action == ACTION_NEW_MSE) {
		install_button->Disable();
		remove_button->Enable(status != STATUS_NOT_INSTALLED);
		upgrade_button->Disable();
		cancel_button->Disable();
	} else if (status == STATUS_INSTALLED) {
		install_button->Disable();
		remove_button->Enable(action != ACTION_UNINSTALL);
		upgrade_button->Disable();
		cancel_button->Enable(action == ACTION_UNINSTALL);
	} else if (status == STATUS_NOT_INSTALLED) {
		install_button->Enable(action != ACTION_INSTALL);
		remove_button->Disable();
		upgrade_button->Disable();
		cancel_button->Enable(action == ACTION_INSTALL);
	} else /* status == STATUS_UPGRADEABLE */ {
		install_button->Disable();
		remove_button->Enable(action != ACTION_UNINSTALL);
		upgrade_button->Enable(action != ACTION_UPGRADE);
		cancel_button->Enable(action != ACTION_NOTHING);
	}
}

void UpdatesWindow::setDefaultPackageStatus() {
	if (!update_version_data) return;
	FOR_EACH(p, update_version_data->packages) {
		PackagedP pack;
		try { pack = packages.openAny(p->name, true); }
		catch (const PackageError&) { } // We couldn't open a package... wonder why?
		
		if (!pack) {
			// not installed
			if (p->app_version > file_version) {
				package_data[p] = PackageData(STATUS_NOT_INSTALLED, ACTION_NEW_MSE);
			} else {
				package_data[p] = PackageData(STATUS_NOT_INSTALLED, ACTION_NOTHING);
			}
		} else if (pack->version < p->version) {
			// newer version
			if (p->app_version > file_version) {
				package_data[p] = PackageData(STATUS_UPGRADEABLE, ACTION_NEW_MSE);
			} else {
				package_data[p] = PackageData(STATUS_UPGRADEABLE, ACTION_UPGRADE);
			}
		} else {
			package_data[p] = PackageData(STATUS_INSTALLED, ACTION_NOTHING);
		}
	}
}

BEGIN_EVENT_TABLE(UpdatesWindow, Frame)
	EVT_COMMAND(wxID_ANY, UPDATE_CHECK_FINISHED_EVT, UpdatesWindow::onUpdateCheckFinished)
	EVT_LISTBOX(ID_PACKAGE_LIST, UpdatesWindow::onPackageSelect)
	EVT_BUTTON(ID_INSTALL, UpdatesWindow::onActionChange)
	EVT_BUTTON(ID_REMOVE,  UpdatesWindow::onActionChange)
	EVT_BUTTON(ID_UPGRADE, UpdatesWindow::onActionChange)
	EVT_BUTTON(ID_CANCEL,  UpdatesWindow::onActionChange)
	EVT_BUTTON(ID_APPLY,   UpdatesWindow::onApplyChanges)
END_EVENT_TABLE()
