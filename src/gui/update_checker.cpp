//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/update_checker.hpp>
#include <gui/packages_window.hpp>
#include <data/settings.hpp>
#include <data/installer.hpp>
#include <util/io/package_manager.hpp>
#include <util/version.hpp>
#include <util/window_id.hpp>
//#include <script/value.hpp> // for some strange reason the profile build needs this :(
//#include <script/to_value.hpp>
#include <wx/dialup.h>
#include <wx/url.h>

DECLARE_POINTER_TYPE(VersionData);
DECLARE_TYPEOF_COLLECTION(PackageDependencyP);

// ----------------------------------------------------------------------------- : Update data

#if !USE_OLD_STYLE_UPDATE_CHECKER

	/// Information on the latest available versions
	class VersionData : public IntrusivePtrBase<VersionData> {
	public:
		vector<PackageDependencyP> packages;  ///< Available packages + versions
		String  new_updates_url;              ///< updates url has moved?
		
		DECLARE_REFLECTION();
	};

	IMPLEMENT_REFLECTION_NO_SCRIPT(VersionData) {
		REFLECT_NO_SCRIPT(packages);
		REFLECT_NO_SCRIPT(new_updates_url);
	}

#else

	/// Information on the latest available version
	class VersionData : public IntrusivePtrBase<VersionData> {
	public:
		Version version;				///< Latest version number of MSE
		String  description;			///< html description of the latest MSE release
		String  new_updates_url;		///< updates url has moved?
		
		DECLARE_REFLECTION();
	};

	IMPLEMENT_REFLECTION(VersionData) {
		REFLECT(version);
		REFLECT(description);
		REFLECT(new_updates_url);
	}

#endif

// The information for the latest version
VersionDataP update_version_data;
// Have we shown the update dialog?
bool shown_dialog = false;
// Is update checking in progress?
volatile bool checking_updates = false;

bool update_data_found() { return !!update_version_data; }
bool update_available()  {
	if (!update_version_data) return false;
	#if !USE_OLD_STYLE_UPDATE_CHECKER
		// updates to any installed package?
		FOR_EACH_CONST(p, update_version_data->packages) {
			if (!settings.check_updates_all && p->package != mse_package) continue;
			Version v;
			if (package_manager.installedVersion(p->package, v) && v < p->version) {
				return true;
			}
		}
		return false;
	#else
		return update_version_data->version > app_version;
	#endif
}

// ----------------------------------------------------------------------------- : Update checking

// Thread to retrieve update information
// Checks if the current version is the latest version
// If not, displays a message
class CheckUpdateThread : public wxThread {
  public:
	virtual void* Entry() {
		Work();
		return 0;
	}
	
	static void Work() {
		if (checking_updates) return; // don't check multiple times simultaniously
		checking_updates = true;
		try {
			#if !USE_OLD_STYLE_UPDATE_CHECKER
				String& the_url = settings.package_versions_url;
			#else
				String& the_url = settings.updates_url;
			#endif
			wxURL url(the_url);
			wxInputStream* isP = url.GetInputStream();
			if (!isP) return; // failed to get data
			InputStreamP is(isP);
			// Read version data
			// ignore errors for forwards compatability
			VersionDataP version_data;
			Reader reader(is, nullptr, _("updates"), true);
			reader.handle(version_data);
			// has the updates url changed?
			if (!version_data->new_updates_url.empty()) {
				the_url = version_data->new_updates_url;
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
#if !USE_OLD_STYLE_UPDATE_CHECKER

void show_update_dialog(Window* parent) {
	if (!update_available() || shown_dialog) return; // we already have the latest version, or this has already been displayed.
	shown_dialog = true;
	(new PackagesWindow(parent))->Show();
}

// ----------------------------------------------------------------------------- : Dialog (old style)
#else // !USE_OLD_STYLE_UPDATE_CHECKER

#include <wx/html/htmlwin.h>

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
	wxDialog* dlg = new wxDialog(parent, wxID_ANY, _TITLE_("updates available"), wxDefaultPosition);
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

#endif // !USE_OLD_STYLE_UPDATE_CHECKER
