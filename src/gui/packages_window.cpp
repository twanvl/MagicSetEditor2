//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/packages_window.hpp>
#include <gui/package_update_list.hpp>
#include <gui/util.hpp>
#include <util/io/package_manager.hpp>
#include <util/window_id.hpp>
#include <data/installer.hpp>
#include <data/settings.hpp>
#include <gfx/gfx.hpp>
#include <wx/wfstream.h>
#include <wx/html/htmlwin.h>
#include <wx/dialup.h>
#include <wx/url.h>
#include <wx/dcbuffer.h>
#include <wx/progdlg.h>
#include <wx/tglbtn.h>

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
	
	/// start downloading, return true if we are done
	bool download();
	
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

bool DownloadableInstallerList::download() {
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
	void draw(DC&);
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
	try {
		draw(dc);
	} CATCH_ALL_ERRORS(false); // don't show message boxes in onPaint!
}
void PackageInfoPanel::draw(DC& dc) {
	wxSize cs = GetClientSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	dc.DrawRectangle(0,0,cs.x,cs.y);
	// draw package info
	if (!package) return;
	PackageDescription& d = *package->description;
	// some borders
	//%int width = cs.x - 10, height = cs.y - 10;
	int x = 5, y = 5;
	// draw icon
	if (d.icon.Ok()) {
		int max_size = 105;
		Image icon = d.icon;
		int icon_w = icon.GetWidth();
		int icon_h = icon.GetHeight();
		if (icon_w <= 20 && icon_h <= 20) {
			// upsample
			icon = resample_preserve_aspect(icon, 96, 96);
			icon_w = icon.GetWidth();
			icon_h = icon.GetHeight();
		}
		dc.DrawBitmap(icon, x+(max_size-icon_w)/2, y+(max_size-icon_h)/2);
		x += max_size;
	}
	// package name
	x += 7;
	dc.SetFont(wxFont(16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _("Arial")));
	dc.DrawText(d.full_name, x, y);
	y += dc.GetCharHeight() + 7;
	
	// version
	dc.SetFont(*wxNORMAL_FONT);
	int dy = dc.GetCharHeight() + 3;
	dc.DrawText(_LABEL_("installed version"),   x, y);
	dc.DrawText(_LABEL_("installable version"), x, y + 1*dy);
	//dc.DrawText(_LABEL_("installer size"),      x, y + 2*dy);
	//dc.DrawText(_LABEL_("installer status"),    x, y + 3*dy);
	// text size?
	int dx = 0, max_dx = 0;
	dc.GetTextExtent(_LABEL_("installed version"),   &dx, nullptr); max_dx = max(max_dx, dx);
	dc.GetTextExtent(_LABEL_("installable version"), &dx, nullptr); max_dx = max(max_dx, dx);
	//dc.GetTextExtent(_LABEL_("installer size"),      &dx, nullptr); max_dx = max(max_dx, dx);
	//dc.GetTextExtent(_LABEL_("installer status"),    &dx, nullptr); max_dx = max(max_dx, dx);
	x += max_dx + 5;
	dc.DrawText(package->installed ? package->installed->version.toString()   : _LABEL_("no version"), x, y);
	dc.DrawText(package->installer ? package->description->version.toString() : _LABEL_("no version"), x, y + 1*dy);
	//dc.DrawText(_("?"), x, y + 2*dy);
	//dc.DrawText(_("?"), x, y + 3*dy);
}

wxSize PackageInfoPanel::DoGetBestSize() const {
	return wxSize(200, 120);
}

BEGIN_EVENT_TABLE(PackageInfoPanel, wxPanel)
	EVT_PAINT(PackageInfoPanel::onPaint)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------- : PackagesWindow

enum Action {
	KEEP, INSTALL, UPGRADE, REMOVE
};

PackagesWindow::PackagesWindow(Window* parent, bool download_package_list)
	: waiting_for_list(download_package_list)
{
	// request download before searching disk so we do two things at once
	if (download_package_list) downloadable_installers.download();
	init(parent, false);
}
PackagesWindow::PackagesWindow(Window* parent, const InstallerP& installer)
	: waiting_for_list(false)
{
	init(parent, true);
	// add installer
	merge(installable_packages, new_intrusive1<DownloadableInstaller>(installer));
	FOR_EACH(p, installable_packages) p->determineStatus();
	// mark all packages in the installer for installation
	FOR_EACH(ip, installable_packages) {
		if (ip->can(PACKAGE_ACT_INSTALL)) {
			set_package_action(installable_packages, ip, PACKAGE_ACT_INSTALL | where);
		}
	}
	// update window
	package_list->rebuild();
	package_list->expandAll();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void PackagesWindow::init(Window* parent, bool show_only_installable) {
	where = is_install_local(settings.install_type) ? PACKAGE_ACT_LOCAL : PACKAGE_ACT_GLOBAL;
	Create(parent, wxID_ANY, _TITLE_("packages window"), wxDefaultPosition, wxSize(640,580), wxDEFAULT_DIALOG_STYLE | wxCLIP_CHILDREN | wxRESIZE_BORDER);
	
	// get packages
	wxBusyCursor busy;
	package_manager.findAllInstalledPackages(installable_packages);
	FOR_EACH(p, installable_packages) p->determineStatus();
	checkInstallerList(false);
	
	// ui elements
	SetIcon(wxIcon());
	package_list = new PackageUpdateList(this, installable_packages, show_only_installable, ID_PACKAGE_LIST);
	package_info = new PackageInfoPanel(this);
	
	wxToggleButton* keep_button    = new wxToggleButton(this, ID_KEEP,    _BUTTON_("keep package"));
	wxToggleButton* install_button = new wxToggleButton(this, ID_INSTALL, _BUTTON_("install package"));
	wxToggleButton* remove_button  = new wxToggleButton(this, ID_REMOVE,  _BUTTON_("remove package"));
	/*
	wxRadioButton* keep_button    = new wxRadioButton(this, ID_KEEP,    _BUTTON_("keep package"));
	wxRadioButton* install_button = new wxRadioButton(this, ID_INSTALL, _BUTTON_("install package"));
	wxRadioButton* upgrade_button = new wxRadioButton(this, ID_UPGRADE, _BUTTON_("upgrade package"));
	wxRadioButton* remove_button  = new wxRadioButton(this, ID_REMOVE,  _BUTTON_("remove package"));
	*/
	
	// Init sizer
	wxBoxSizer* v = new wxBoxSizer(wxVERTICAL);
		v->Add(package_list, 1, wxEXPAND | wxALL & ~wxBOTTOM, 8);
		v->AddSpacer(4);
		wxBoxSizer* h = new wxBoxSizer(wxHORIZONTAL);
			h->Add(package_info, 1, wxRIGHT, 4);
			wxBoxSizer* v2 = new wxBoxSizer(wxVERTICAL);
				v2->Add(install_button, 0, wxEXPAND | wxBOTTOM, 4);
				v2->AddStretchSpacer();
				v2->Add(keep_button,    0, wxEXPAND | wxBOTTOM, 4);
				v2->AddStretchSpacer();
				v2->Add(remove_button,  0, wxEXPAND | wxBOTTOM, 0);
			h->Add(v2);
		v->Add(h, 0, wxEXPAND | wxALL & ~wxTOP, 8);
		v->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP, 8);
	SetSizer(v);
	
	wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);
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
		PackageAction act = ev.GetId() == ID_INSTALL ? PACKAGE_ACT_INSTALL
		                  : ev.GetId() == ID_UPGRADE ? PACKAGE_ACT_INSTALL
		                  : ev.GetId() == ID_REMOVE  ? PACKAGE_ACT_REMOVE
		                  : PACKAGE_ACT_NOTHING;
		act = act | where;
		// set action
		set_package_action(installable_packages, package, act);
		package_list->Refresh(false);
		UpdateWindowUI(wxUPDATE_UI_RECURSE);
	}
}

void PackagesWindow::onOk(wxCommandEvent& ev) {
	// Do we need a new version of MSE first?
	// count number of packages to change
	int to_change   = 0;
	int to_download = 0;
	int to_remove   = 0;
	int with_modifications = 0;
	FOR_EACH(ip, installable_packages) {
		if (!ip->has(PACKAGE_ACT_NOTHING)) ++to_change;
		if (ip->has(PACKAGE_ACT_INSTALL) && ip->installer && !ip->installer->installer) ++to_download;
		if (ip->has(PACKAGE_ACT_REMOVE)) {
			to_remove++;
			if (ip->has(PACKAGE_MODIFIED)) with_modifications++;
		}
	}
	// anything to do?
	if (!to_change) {
		ev.Skip();
		return;
	}
	// Warn about removing
	if (to_remove) {
		int result = wxMessageBox(
			with_modifications == 0 ? _ERROR_1_("remove packages",           String()<<to_remove)
			                        : _ERROR_2_("remove packages modified",  String()<<to_remove,  String()<<with_modifications),
			_TITLE_("packages window"), wxICON_EXCLAMATION | wxYES_NO);
		if (result == wxNO) return;
	}
	// progress dialog
	wxProgressDialog progress(
			_TITLE_("installing updates"),
			String::Format(_ERROR_("downloading updates"), 0, to_download),
			to_change + to_download,
			this,
			wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_SMOOTH
		);
	// Clear package list
	package_manager.reset();
	// Download installers
	int package_pos = 0, step = 0;
	FOR_EACH(ip, installable_packages) {
		if (ip->has(PACKAGE_ACT_INSTALL) && ip->installer && !ip->installer->installer) {
			if (!progress.Update(step++, String::Format(_ERROR_("downloading updates"), ++package_pos, to_download))) {
				return; // aborted
			}
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
	package_pos = 0;
	int success = 0, install = 0, remove = 0;
	FOR_EACH(ip, installable_packages) {
		if (ip->has(PACKAGE_ACT_NOTHING)) continue; // package unchanged
		if (!progress.Update(step++, String::Format(_ERROR_("installing updates"), ++package_pos, to_change))) {
			// don't allow abort.
		}
		bool ok = package_manager.install(*ip);
		if (ok) {
			install += ip->has(PACKAGE_ACT_INSTALL) && !ip->installed;
			remove  += ip->has(PACKAGE_ACT_REMOVE);
			success += 1;
		}
	}
	// Done
	progress.Update(step++);
	wxMessageBox(
		install == success ? _ERROR_1_("install packages successful",String()<<success):
		remove  == success ? _ERROR_1_("remove packages successful", String()<<success):
		                     _ERROR_1_("change packages successful", String()<<success),
		_TITLE_("packages window"), wxICON_INFORMATION | wxOK);
	// Continue event propagation into the dialog window so that it closes.
	ev.Skip();
	//%% TODO: will we delete packages?
	//%%       If so, we need to warn with _ERROR_1_("remove packages") and _ERROR_2_("remove packages modified")
	//%%       We probably also refer to the type of package, _TYPE_("package"), for example _TYPE_("locale")
	//%%       NOTE: The above text is for the locale.pl program
}

void PackagesWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	wxToggleButton* w = (wxToggleButton*)ev.GetEventObject();
	switch (ev.GetId()) {
		case ID_KEEP:
			w->SetValue(package && package->has(PACKAGE_ACT_NOTHING));
			w->Enable  (package && package->can(PACKAGE_ACT_NOTHING | where));
			w->SetLabel(package && package->installed ? _BUTTON_("keep package") : _BUTTON_("don't install package"));
			break;
		case ID_INSTALL:
			w->SetValue(package && package->has(PACKAGE_ACT_INSTALL | where));
			w->Enable  (package && package->can(PACKAGE_ACT_INSTALL | where));
			w->SetLabel( !package || !package->installed ? _BUTTON_("install package")
			           :  package->has(PACKAGE_UPDATES)  ? _BUTTON_("upgrade package")
			           :                                   _BUTTON_("reinstall package"));
			break;
		case ID_REMOVE:
			w->SetValue(package && package->has(PACKAGE_ACT_REMOVE  | where));
			w->Enable  (package && package->can(PACKAGE_ACT_REMOVE  | where));
			//w->SetLabel(package && package->... ? _BUTTON_("remove group") : _BUTTON_("remove package"));
			break;
	}
	// TODO: change labels to _BUTTON_("install group"), _BUTTON_("remove group"), _BUTTON_("upgrade group")
}

void PackagesWindow::onIdle(wxIdleEvent& ev) {
	ev.RequestMore(!checkInstallerList());
}

bool PackagesWindow::checkInstallerList(bool refresh) {
	if (!waiting_for_list) return true;
	if (!downloadable_installers.download()) return false;
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
	EVT_TOGGLEBUTTON(ID_KEEP,     PackagesWindow::onActionChange)
	EVT_TOGGLEBUTTON(ID_INSTALL,  PackagesWindow::onActionChange)
	EVT_TOGGLEBUTTON(ID_REMOVE,   PackagesWindow::onActionChange)
	EVT_TOGGLEBUTTON(ID_UPGRADE,  PackagesWindow::onActionChange)
	EVT_BUTTON(wxID_OK,     PackagesWindow::onOk)
	EVT_UPDATE_UI(wxID_ANY, PackagesWindow::onUpdateUI)
	EVT_IDLE  (             PackagesWindow::onIdle)
END_EVENT_TABLE()
