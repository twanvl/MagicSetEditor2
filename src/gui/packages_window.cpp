//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/packages_window.hpp>
#include <gui/control/tree_list.hpp>
//%#include <gui/update_checker.hpp>
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

DECLARE_POINTER_TYPE(Installer);

DECLARE_TYPEOF_COLLECTION(PackageDependencyP);
DECLARE_TYPEOF_COLLECTION(InstallablePackageP);
DECLARE_TYPEOF_COLLECTION(DownloadableInstallerP);
DECLARE_TYPEOF_COLLECTION(TreeList::ItemP);

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

// ----------------------------------------------------------------------------- : PackageUpdateList

/// A list of installed and downloadable packages
class PackageUpdateList : public TreeList {
  public:
	PackageUpdateList(PackagesWindow* parent, int id = wxID_ANY)
		: TreeList(parent, id)
		, parent(parent)
	{
		item_height = max(item_height,17);
		rebuild();
	}
	
	InstallablePackageP getSelection() const {
		return selection == NOTHING ? InstallablePackageP() : get(selection);
	}
	
	InstallablePackageP get(size_t item) const {
		return static_pointer_cast<TreeItem>(items[item])->package;
	}
		
  protected:
	virtual void initItems();
	virtual void drawItem(DC& dc, size_t index, size_t column, int x, int y, bool selected) const;
	
	virtual size_t columnCount() const { return 3; }
	virtual String columnText(size_t column) const;
	virtual int    columnWidth(size_t column) const;
	
  private:
	PackagesWindow* parent;
	class TreeItem;
  public:
	typedef intrusive_ptr<TreeItem> TreeItemP;
  private:
	class TreeItem : public Item {
	  public:
		String label;
		vector<TreeItemP> children;
		InstallablePackageP package;
		Bitmap icon, icon_grey;
		
		void add(const InstallablePackageP& package, const String& path, int level = -1);
		void toItems(vector<TreeList::ItemP>& items);
		bool highlight() const;
	};
};

DECLARE_TYPEOF_COLLECTION(PackageUpdateList::TreeItemP);


void PackageUpdateList::TreeItem::add(const InstallablePackageP& package, const String& path, int level) {
	size_t pos = path.find_first_of(_('/'));
	String name = path.substr(0,pos);
	String rest = pos == String::npos ? _("") : path.substr(pos+1);
	// find/add child
	FOR_EACH(ti, children) {
		if (ti->label == name) {
			// already have this child
			if (pos == String::npos) {
				if (ti->package) {
					// two packages with the same path
					TreeItemP ti2(new TreeItem);
					ti2->level = level + 1;
					ti2->label = name;
					ti2->package = package;
					children.insert(ti_IT.first, ti2);
				} else {
					ti->package = package;
				}
			} else {
				ti->add(package, rest, level + 1);
			}
			return;
		}
	}
	// don't have this child
	TreeItemP ti(new TreeItem);
	children.push_back(ti);
	ti->level = level + 1;
	ti->label = name;
	if (pos == String::npos) {
		ti->package = package;
	} else {
		ti->add(package, rest, level + 1);
	}
}

bool compare_pos_hint(const PackageUpdateList::TreeItemP& a, const PackageUpdateList::TreeItemP& b) {
	int pa = a->package ? a->package->description->position_hint : 0;
	int pb = b->package ? b->package->description->position_hint : 0;
	if (pa < pb) return true;
	if (pa > pb) return false;
	return a->label < b->label;
}

void PackageUpdateList::TreeItem::toItems(vector<TreeList::ItemP>& items) {
	sort(children.begin(), children.end(), compare_pos_hint);
	FOR_EACH(c, children) {
		items.push_back(c);
		c->toItems(items);
	}
}

bool PackageUpdateList::TreeItem::highlight() const {
	if (package && ((package->installed && !(package->action & PACKAGE_REMOVE))
	               || package->action & (PACKAGE_INSTALL | PACKAGE_UPGRADE))) {
		return true;
	}
	FOR_EACH_CONST(c,children) if (c->highlight()) return true;
	return false;
}


void PackageUpdateList::initItems() {
	// packages to tree
	TreeItem root;
	FOR_EACH(ip, parent->installable_packages) {
		String group = ip->description->installer_group;
		if (!group.empty()) group += _("/");
		group += ip->description->short_name;
		root.add(ip, group);
	}
	// tree to treelist items
	items.clear();
	root.toItems(items);
	// init image list
	FOR_EACH(i,items) {
		TreeItem& ti = static_cast<TreeItem&>(*i);
		const InstallablePackageP& p = ti.package;
		Image image;
		if (p && p->description->icon.Ok()) {
			Image resampled(16,16,false);
			resample_preserve_aspect(p->description->icon, resampled);
			image = resampled;
		} else if (p) {
			image = load_resource_image(_("installer_package"));
		} else if (ti.label == _("locales")) {
			image = load_resource_image(_("installer_locales"));
		} else {
			image = load_resource_image(_("installer_group"));
		}
		ti.icon = Bitmap(image);
		desaturate(image);
		set_alpha(image, 0.5);
		ti.icon_grey = Bitmap(image);
	}
}

void PackageUpdateList::drawItem(DC& dc, size_t index, size_t column, int x, int y, bool selected) const {
	const TreeItem& ti = static_cast<const TreeItem&>(*items[index]);
	Color color = wxSystemSettings::GetColour(selected ? wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT);
	if (column == 0) {
		// Name
		const Bitmap& bmp = ti.highlight() ? ti.icon : ti.icon_grey;
		if (bmp.Ok()) dc.DrawBitmap(bmp,x,y);
		dc.SetTextForeground(color);
		dc.DrawText(ti.label, x+18, y+2);
	} else if (column == 1 && ti.package) {
		// Status
		int stat = ti.package->status;
		if ((stat & PACKAGE_CONFLICTS) == PACKAGE_CONFLICTS) {
			dc.SetTextForeground(lerp(color,Color(255,0,0),0.8));
			dc.DrawText(_LABEL_("package conflicts"), x+1,y+2);
		} else if ((stat & PACKAGE_MODIFIED) == PACKAGE_MODIFIED) {
			dc.SetTextForeground(lerp(color,Color(255,255,0),0.5));
			dc.DrawText(_LABEL_("package modified"), x+1,y+2);
		} else if ((stat & PACKAGE_UPDATES) == PACKAGE_UPDATES) {
			dc.SetTextForeground(lerp(color,Color(0,0,255),0.5));
			dc.DrawText(_LABEL_("package updates"), x+1,y+2);
		} else if ((stat & PACKAGE_INSTALLED) == PACKAGE_INSTALLED) {
			dc.SetTextForeground(color);
			dc.DrawText(_LABEL_("package installed"), x+1,y+2);
		} else if ((stat & PACKAGE_INSTALLABLE) == PACKAGE_INSTALLABLE) {
			dc.SetTextForeground(lerp(color,Color(128,128,128),0.6));
			dc.SetTextForeground(color);
			dc.DrawText(_LABEL_("package installable"), x+1,y+2);
		}
	} else if (column == 2 && ti.package) {
		// Action
		int act = ti.package->action;
		if (act & PACKAGE_INSTALL) {
			dc.SetTextForeground(lerp(color,Color(0,255,0),0.5));
			dc.DrawText(_LABEL_("install package"), x+1,y+2);
		} else if (act & PACKAGE_UPGRADE) {
			dc.SetTextForeground(lerp(color,Color(0,0,255),0.5));
			dc.DrawText(_LABEL_("upgrade package"), x+1,y+2);
		} else if (act & PACKAGE_REMOVE) {
			dc.SetTextForeground(lerp(color,Color(255,0,0),0.5));
			dc.DrawText(_LABEL_("remove package"), x+1,y+2);
		}
	}
}

String PackageUpdateList::columnText(size_t column) const {
	if       (column == 0) return _LABEL_("package name");
	else if  (column == 1) return _LABEL_("package status");
	else if  (column == 2) return _LABEL_("package action");
	else                   throw InternalError(_("Unknown column"));
}
int PackageUpdateList::columnWidth(size_t column) const {
	if (column == 0) {
		wxSize cs = GetClientSize();
		return cs.x - 300;
	} else {
		return 150;
	}
}


// ----------------------------------------------------------------------------- : PackageInfoPanel

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
	: wxDialog(parent, wxID_ANY, _TITLE_("package list"), wxDefaultPosition, wxSize(640,480), wxDEFAULT_DIALOG_STYLE | wxCLIP_CHILDREN | wxRESIZE_BORDER)
	, where(is_install_local(settings.install_type) ? PACKAGE_LOCAL : PACKAGE_GLOBAL)
	, waiting_for_list(download_package_list)
{
	// request download before searching disk so we do two things at once
	if (download_package_list) downloadable_installers.done();
	// get packages
	wxBusyCursor busy;
	packages.installedPackages(installable_packages);
	FOR_EACH(p, installable_packages) p->determineStatus();
	checkInstallerList();
	
	// ui elements
	SetIcon(wxIcon());
	package_list = new PackageUpdateList(this, ID_PACKAGE_LIST);
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
		                  : ev.GetId() == ID_UPGRADE ? PACKAGE_UPGRADE
		                  : ev.GetId() == ID_REMOVE  ? PACKAGE_REMOVE
		                  : PACKAGE_NOTHING;
		act = (PackageAction)(act | where);
		// toggle action
		if (package->has(act)) {
			set_package_action(installable_packages, package, (PackageAction)(PACKAGE_NOTHING | where));
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
		if ((ip->action & (PACKAGE_INSTALL | PACKAGE_UPGRADE)) && ip->installer && !ip->installer->installer) {
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
	wxDialog::OnOK(ev);
}

void PackagesWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_INSTALL:
		case ID_UPGRADE:
		case ID_REMOVE: {
			PackageAction act = ev.GetId() == ID_INSTALL ? PACKAGE_INSTALL
			                  : ev.GetId() == ID_UPGRADE ? PACKAGE_UPGRADE
			                  : ev.GetId() == ID_REMOVE  ? PACKAGE_REMOVE
			                  : PACKAGE_NOTHING;
			act = (PackageAction)(act | where);
			ev.Check (package && package->has(act));
			ev.Enable(package && package->can(act));
			break;
		}
	}
}

void PackagesWindow::onIdle(wxIdleEvent& ev) {
	ev.RequestMore(!checkInstallerList());
}

bool PackagesWindow::checkInstallerList() {
	if (!waiting_for_list) return true;
	if (!downloadable_installers.done()) return false;
	waiting_for_list = false;
	// merge installer lists
	FOR_EACH(inst, downloadable_installers.installers) {
		merge(installable_packages, inst);
	}
	FOR_EACH(p, installable_packages) p->determineStatus();
	// refresh
	package_list->rebuild();
	package_info->setPackage(package = package_list->getSelection());
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
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
