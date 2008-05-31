//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/packages_window.hpp>
#include <gui/package_update_list.hpp>
#include <util/io/package_manager.hpp>
#include <util/window_id.hpp>
#include <data/installer.hpp>
#include <data/settings.hpp>
#include <wx/wfstream.h>
#include <wx/html/htmlwin.h>
#include <wx/dialup.h>
#include <wx/url.h>
#include <wx/dcbuffer.h>
#include <wx/progdlg.h>

DECLARE_POINTER_TYPE(Installer);

DECLARE_TYPEOF_COLLECTION(PackageDependencyP);
DECLARE_TYPEOF_COLLECTION(InstallablePackageP);
DECLARE_TYPEOF_COLLECTION(DownloadableInstallerP);

// ----------------------------------------------------------------------------- : TODO: MOVE

/*
// A HTML control that opens all pages in an actual browser
struct HtmlWindowToBrowser : public wxHtmlWindow {
	HtmlWindowToBrowser(Window* parent, int id, const wxPoint& pos, const wxSize& size, long flags)
		: wxHtmlWindow(parent, id, pos, size, flags)
	{}
		
	virtual void OnLinkClicked(const wxHtmlLinkInfo& info) {
		wxLaunchDefaultBrowser( info.GetHref() );
	}
};
*/

// ----------------------------------------------------------------------------- : DownloadableInstallers

/// Handle downloading of installers
class DownloadableInstallerList {
  public:
	DownloadableInstallerList() : status(NONE) {}
	
	/// are we done? if not, start downloading
	bool done();
	
	vector<DownloadableInstallerP> installers;
	
  private:
	enum Status { NONE, DOWNLOADING, DONE } status;
	wxMutex lock;
	
	struct Thread : public wxThread {
		virtual ExitCode Entry();
	};
};

/// The global installer downloader
DownloadableInstallerList downloadable_installers;

bool DownloadableInstallerList::done() {
	if (status == DONE) return true;
	if (status == NONE) {
		status = DOWNLOADING;
		Thread* thread = new Thread();
		thread->Create();
		thread->Run();
	}
	return false;
}

wxThread::ExitCode DownloadableInstallerList::Thread::Entry() {
	// open url
	wxURL url(settings.installer_list_url);
	wxInputStream* isP = url.GetInputStream();
	if (!isP) {
		wxMutexLocker l(downloadable_installers.lock);
		downloadable_installers.status = DONE;
		return 0;
	}
	InputStreamP is(isP);
	// Read installer list
	Reader reader(is, nullptr, _("installers"), true);
	vector<DownloadableInstallerP> installers;
	reader.handle(_("installers"),installers);
	// done
	wxMutexLocker l(downloadable_installers.lock);
	swap(installers, downloadable_installers.installers);
	downloadable_installers.status = DONE;
	return 0;
}

// ----------------------------------------------------------------------------- : PackageInfoPanel

/// Information on a package
class PackageInfoPanel : public wxPanel {
  public:
	PackageInfoPanel(Window* parent);
	
	void setPackage(const InstallablePackageP& package);
	
	virtual wxSize DoGetBestSize() const;
  private:
	InstallablePackageP package;
	
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
};

PackageInfoPanel::PackageInfoPanel(Window* parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{}

void PackageInfoPanel::setPackage(const InstallablePackageP& package) {
	this->package = package;
	Refresh(false);
}

void PackageInfoPanel::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	wxSize cs = GetClientSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	dc.DrawRectangle(0,0,cs.x,cs.y);
	// draw package info
	if (!package) return;
	PackageDescription& d = *package->description;
	int x = 5;
	if (d.icon.Ok()) {
		int h = d.icon.GetHeight();
		int y = max(0,20-h)/2 + 5;
		dc.DrawBitmap(d.icon, x,y);
		x += d.icon.GetWidth();
	}
	dc.SetFont(wxFont(16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _("Arial")));
	dc.DrawText(d.full_name, x + 5, 3);
}

wxSize PackageInfoPanel::DoGetBestSize() const {
	return wxSize(200, 120);
}

BEGIN_EVENT_TABLE(PackageInfoPanel, wxPanel)
	EVT_PAINT(PackageInfoPanel::onPaint)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------- : PackagesWindow

PackagesWindow::PackagesWindow(Window* parent, bool download_package_list)
	: wxDialog(parent, wxID_ANY, _TITLE_("packages window"), wxDefaultPosition, wxSize(640,480), wxDEFAULT_DIALOG_STYLE | wxCLIP_CHILDREN | wxRESIZE_BORDER)
	, where(is_install_local(settings.install_type) ? PACKAGE_LOCAL : PACKAGE_GLOBAL)
	, waiting_for_list(download_package_list)
{
	// request download before searching disk so we do two things at once
	if (download_package_list) downloadable_installers.done();
	// get packages
	wxBusyCursor busy;
	packages.installedPackages(installable_packages);
	FOR_EACH(p, installable_packages) p->determineStatus();
	checkInstallerList(false);
	
	// ui elements
	SetIcon(wxIcon());
	package_list = new PackageUpdateList(this, installable_packages, ID_PACKAGE_LIST);
	package_info = new PackageInfoPanel(this);
	
	//wxToolbar* buttons = new wxToolbar
	wxButton* install_button = new wxButton(this, ID_INSTALL, _BUTTON_("install package"));
	wxButton* upgrade_button = new wxButton(this, ID_UPGRADE, _BUTTON_("upgrade package"));
	wxButton* remove_button  = new wxButton(this, ID_REMOVE,  _BUTTON_("remove package"));
	
	// Init sizer
	wxBoxSizer* v = new wxBoxSizer(wxVERTICAL);
		v->Add(package_list, 1, wxEXPAND | wxALL & ~wxBOTTOM, 8);
		v->AddSpacer(4);
		wxBoxSizer* h = new wxBoxSizer(wxHORIZONTAL);
			h->Add(package_info, 1, wxRIGHT, 4);
			wxBoxSizer* v2 = new wxBoxSizer(wxVERTICAL);
				v2->Add(install_button, 0, wxEXPAND | wxBOTTOM, 4);
				v2->Add(upgrade_button, 0, wxEXPAND | wxBOTTOM, 4);
				v2->Add(remove_button,  0, wxEXPAND | wxBOTTOM, 4);
				v2->AddStretchSpacer();
			h->Add(v2);
		v->Add(h, 0, wxEXPAND | wxALL & ~wxTOP, 8);
		v->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP, 8);
	SetSizer(v);
	
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

PackagesWindow::~PackagesWindow() {
}


void PackagesWindow::onPackageSelect(wxCommandEvent& ev) {
	package_info->setPackage(package = package_list->getSelection());
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void PackagesWindow::onActionChange(wxCommandEvent& ev) {
	if (package) {
		PackageAction act = ev.GetId() == ID_INSTALL ? PACKAGE_INSTALL
		                  : ev.GetId() == ID_UPGRADE ? PACKAGE_INSTALL
		                  : ev.GetId() == ID_REMOVE  ? PACKAGE_REMOVE
		                  : PACKAGE_NOTHING;
		act = act | where;
		// toggle action
		if (package->has(act)) {
			set_package_action(installable_packages, package, PACKAGE_NOTHING | where);
		} else {
			set_package_action(installable_packages, package, act);
		}
		package_list->Refresh(false);
		UpdateWindowUI(wxUPDATE_UI_RECURSE);
	}
}

void PackagesWindow::onOk(wxCommandEvent& ev) {
	// Do we need a new version of MSE first?
	// progress dialog
	int total = (int)installable_packages.size();
	wxProgressDialog progress(
			_TITLE_("installing updates"),
			String::Format(_ERROR_("downloading updates"), 0, 2*total),
			total,
			this,
			wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_SMOOTH
		);
	// Clear package list
	packages.reset();
	// Download installers
	int n = 0;
	FOR_EACH(ip, installable_packages) {
		++n;
		if (!progress.Update(n, String::Format(_ERROR_("downloading updates"), n, total))) {
			return; // aborted
		}
		if ((ip->action & PACKAGE_INSTALL) && ip->installer && !ip->installer->installer) {
			// download installer
			wxURL url(ip->installer->installer_url);
			wxInputStream* is = url.GetInputStream();
			if (!is) {
				throw Error(_ERROR_2_("can't download installer", ip->description->name, ip->installer->installer_url));
			}
			ip->installer->installer_file = wxFileName::CreateTempFileName(_("mse-installer"));
			wxFileOutputStream os(ip->installer->installer_file);
			os.Write(*is);
			os.Close();
			// open installer
			ip->installer->installer = new_intrusive<Installer>();
			ip->installer->installer->open(ip->installer->installer_file);
		}
	}
	// Install stuff
	int n2 = 0 ;
	FOR_EACH(ip, installable_packages) {
		++n; ++n2;
		if (!progress.Update(n, String::Format(_ERROR_("installing updates"), n2, total))) {
			// don't allow abort.
		}
		packages.install(*ip);
	}
	// Done
	// Continue event propagation into the dialog window so that it closes.
	ev.Skip();
	//%% TODO: will we delete packages?
	//%%       If so, we need to warn with _ERROR_1_("remove packages") and _ERROR_2_("remove packages modified")
	//%%       We probably also refer to the type of package, _TYPE_("package"), for example _TYPE_("locale")
	//%%       NOTE: The above text is for the locale.pl program
}

void PackagesWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_INSTALL:
			ev.Check (package && package->has(PACKAGE_INSTALL | where) && !package->installed);
			ev.Enable(package && package->can(PACKAGE_INSTALL | where) && !package->installed);
			break;
		case ID_UPGRADE:
			ev.Check (package && package->has(PACKAGE_INSTALL | where) && package->installed);
			ev.Enable(package && package->can(PACKAGE_INSTALL | where) && package->installed);
			break;
		case ID_REMOVE:
			ev.Check (package && package->has(PACKAGE_REMOVE  | where));
			ev.Enable(package && package->can(PACKAGE_REMOVE  | where));
			break;
	}
	// TODO: change labels to _BUTTON_("install group"), _BUTTON_("remove group"), _BUTTON_("upgrade group")
}

void PackagesWindow::onIdle(wxIdleEvent& ev) {
	ev.RequestMore(!checkInstallerList());
}

bool PackagesWindow::checkInstallerList(bool refresh) {
	if (!waiting_for_list) return true;
	if (!downloadable_installers.done()) return false;
	waiting_for_list = false;
	// merge installer lists
	FOR_EACH(inst, downloadable_installers.installers) {
		merge(installable_packages, inst);
	}
	FOR_EACH(p, installable_packages) p->determineStatus();
	// refresh
	if (refresh) {
		package_list->rebuild();
		package_info->setPackage(package = package_list->getSelection());
		UpdateWindowUI(wxUPDATE_UI_RECURSE);
	}
	return true;
}

BEGIN_EVENT_TABLE(PackagesWindow, wxDialog)
	EVT_LISTBOX(ID_PACKAGE_LIST, PackagesWindow::onPackageSelect)
	EVT_BUTTON(ID_INSTALL,  PackagesWindow::onActionChange)
	EVT_BUTTON(ID_REMOVE,   PackagesWindow::onActionChange)
	EVT_BUTTON(ID_UPGRADE,  PackagesWindow::onActionChange)
	EVT_BUTTON(wxID_OK,     PackagesWindow::onOk)
	EVT_UPDATE_UI(wxID_ANY, PackagesWindow::onUpdateUI)
	EVT_IDLE  (             PackagesWindow::onIdle)
END_EVENT_TABLE()
