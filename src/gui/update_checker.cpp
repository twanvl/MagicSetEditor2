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
#include <script/value.hpp> // for some strange reason the profile build needs this :(
#include <wx/dialup.h>
#include <wx/url.h>
#include <wx/html/htmlwin.h>

DECLARE_POINTER_TYPE(PackageVersionData);
DECLARE_POINTER_TYPE(VersionData);

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
	vector<PackageDependencyP> depends;	///< Packages this depends on
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

IMPLEMENT_REFLECTION(VersionData) {
	REFLECT(version);
	REFLECT(description);
	REFLECT(new_updates_url);
}

// The information for the latest version
VersionDataP update_version_data;
// Is update checking in progress?
volatile bool checking_updates = false;

bool update_data_found() { return !!update_version_data; }
bool update_available()  { return update_version_data && update_version_data->version > app_version; }

// ----------------------------------------------------------------------------- : Update checking

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
	if (!update_available()) return; // we already have the latest version
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
	update_version_data = VersionDataP();
}
