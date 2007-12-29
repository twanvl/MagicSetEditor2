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
#include <util/io/package_repository.hpp>
#include <util/io/package_manager.hpp>
#include <util/window_id.hpp>
#include <data/installer.hpp>
#include <data/settings.hpp>
#include <gfx/gfx.hpp>
//%#include <list>
//%#include <set>
#include <wx/wfstream.h>
#include <wx/html/htmlwin.h>
#include <wx/dialup.h>
#include <wx/url.h>
#include <wx/dcbuffer.h>
#include <wx/progdlg.h>

//%DECLARE_POINTER_TYPE(VersionData); //%% TODO
DECLARE_POINTER_TYPE(Installer);
//%DECLARE_POINTER_TYPE(PackageVersionData);

DECLARE_TYPEOF_COLLECTION(PackageVersionDataP);
DECLARE_TYPEOF_COLLECTION(PackageDependencyP);
DECLARE_TYPEOF_COLLECTION(InstallablePackageP);
DECLARE_TYPEOF_COLLECTION(TreeList::ItemP);
//%DECLARE_TYPEOF(list<PackageVersionDataP>);
//%DECLARE_TYPEOF(set<String>);

// ----------------------------------------------------------------------------- : TODO: MOVE

/*
/// Information on available packages
class PackageVersionData : public IntrusivePtrVirtualBase {
  public:
	PackageVersionData() {}
	
	String  name;						///< Name of the package
	String  type;						///< Type of package ("magic style" or "game")
	String  display_name;				///< Name to show on package list.
	String  description;				///< html description
	String  url;						///< Where can the package be downloaded?
	Version version;					///< Version number of the download
	Version app_version;				///< The minimium version of MSE required
	vector<PackageDependencyP> depends;	///< Packages this depends on

	DECLARE_REFLECTION();
};

/// Information on the latest available version
class VersionData : public IntrusivePtrBase<VersionData> {
  public:
	Version version;				///< Latest version number of MSE
	String  description;			///< html description of the latest MSE release
	String  new_updates_url;		///< updates url has moved?
	vector<PackageVersionDataP> packages;	///< Available packages
	
	DECLARE_REFLECTION();
};
extern VersionDataP update_version_data;

// A HTML control that opens all pages in an actual browser
struct HtmlWindowToBrowser : public wxHtmlWindow {
	HtmlWindowToBrowser(Window* parent, int id, const wxPoint& pos, const wxSize& size, long flags)
		: wxHtmlWindow(parent, id, pos, size, flags)
	{}
		
	virtual void OnLinkClicked(const wxHtmlLinkInfo& info) {
		wxLaunchDefaultBrowser( info.GetHref() );
	}
};

// ----------------------------------------------------------------------------- : PackageUpdateList

/*
class PackageUpdateList : public wxVListBox {
  public:
	PackageUpdateList(UpdatesWindow* parent)
		: wxVListBox (parent, ID_PACKAGE_LIST, wxDefaultPosition, wxSize(540,210), wxNO_BORDER | wxVSCROLL)
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
			
			dc.SetClippingRegion(wxRect(rect.x, rect.y, 180, rect.height));
			dc.SetTextForeground(SELECT_WHITE(packageFront));
			dc.DrawText(pack->display_name, rect.GetLeft() + 1, rect.GetTop());
			dc.DestroyClippingRegion();
			
			dc.SetClippingRegion(wxRect(rect.x + 180, rect.y, 120, rect.height));
			dc.DrawText(pack->type, rect.GetLeft() + 180, rect.GetTop());
			dc.DestroyClippingRegion();
			
			dc.SetTextForeground(SELECT_WHITE(status_colors[status]));
			dc.DrawText(status_texts[status], rect.GetLeft() + 300, rect.GetTop());
			
			dc.SetTextForeground(SELECT_WHITE(action_colors[action]));
			dc.DrawText(action_texts[action], rect.GetLeft() + 420, rect.GetTop());
			
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
*/


// ----------------------------------------------------------------------------- : PackageUpdateList

/// A list of installed and downloadable packages
class PackageUpdateList : public TreeList {
  public:
	PackageUpdateList(PackagesWindow* parent, int id = wxID_ANY)
		: TreeList(parent, id)
		, parent(parent)
		, images(16,16)
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
	mutable wxImageList images;
	class TreeItem;
  public:
	typedef intrusive_ptr<TreeItem> TreeItemP;
  private:
	class TreeItem : public Item {
	  public:
		String label;
		vector<TreeItemP> children;
		InstallablePackageP package;
		
		void add(const InstallablePackageP& package, const String& path, int level = -1);
		void toItems(vector<TreeList::ItemP>& items);
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


void PackageUpdateList::initItems() {
	// packages to tree
	TreeItem root;
	FOR_EACH(ip, parent->installable_packages) {
		String group = ip->description->installer_group;
//%		if (group.empty()) group = _("Custom");
//%		group += _("/");
		if (!group.empty()) group += _("/");
		group += ip->description->short_name;
		root.add(ip, group);
	}
	// tree to treelist items
	items.clear();
	root.toItems(items);
	// init image list
	images.RemoveAll();
	Image resampled(16,16,false);
	FOR_EACH(i,items) {
		const TreeItem& ti = static_cast<const TreeItem&>(*i);
		const InstallablePackageP& p = ti.package;
		if (p && p->description->icon.Ok()) {
			resample_preserve_aspect(p->description->icon, resampled);
			images.Add(resampled);
		} else if (p) {
			images.Add(load_resource_image(_("installer_package")));
		} else if (ti.label == _("locales")) {
			images.Add(load_resource_image(_("installer_locales")));
		} else {
			images.Add(load_resource_image(_("installer_group")));
		}
	}
}

void PackageUpdateList::drawItem(DC& dc, size_t index, size_t column, int x, int y, bool selected) const {
	const TreeItem& ti = static_cast<const TreeItem&>(*items[index]);
	Color color = wxSystemSettings::GetColour(selected ? wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT);
	if (column == 0) {
		// Name
		images.Draw((int)index, dc, x, y);
		dc.SetTextForeground(color);
		dc.DrawText(ti.label, x+18, y+2);
	} else if (column == 1 && ti.package) {
		// Status
		int stat = ti.package->status;
		if ((stat & PACKAGE_CONFLICTS) == PACKAGE_CONFLICTS) {
			dc.DrawText(_LABEL_("package conflicts"), x+1,y+2);
		} else if ((stat & PACKAGE_MODIFIED) == PACKAGE_MODIFIED) {
			dc.DrawText(_LABEL_("package modified"), x+1,y+2);
		} else if ((stat & PACKAGE_UPDATES) == PACKAGE_UPDATES) {
			dc.DrawText(_LABEL_("package updates"), x+1,y+2);
		} else if ((stat & PACKAGE_INSTALLED) == PACKAGE_INSTALLED) {
			dc.DrawText(_LABEL_("package installed"), x+1,y+2);
		} else if ((stat & PACKAGE_INSTALLABLE) == PACKAGE_INSTALLABLE) {
			dc.DrawText(_LABEL_("package installable"), x+1,y+2);
		}
	} else if (column == 2 && ti.package) {
		// Action
		int act = ti.package->action;
		if (act & PACKAGE_INSTALL) {
			dc.DrawText(_LABEL_("install package"), x+1,y+2);
		} else if (act & PACKAGE_UPGRADE) {
			dc.DrawText(_LABEL_("upgrade package"), x+1,y+2);
		} else if (act & PACKAGE_REMOVE) {
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
{
	// get packages
	wxBusyCursor busy;
	packages.installedPackages(installable_packages);
	FOR_EACH(p, installable_packages) p->determineStatus();
	// ui elements
	SetIcon(wxIcon());
	package_list = new PackageUpdateList(this, ID_PACKAGE_LIST);
	package_info = new PackageInfoPanel(this);
	
	//wxToolbar* buttons = new wxToolbar
	wxButton* install_button = new wxButton(this, ID_INSTALL, _BUTTON_("install package"));
	wxButton* upgrade_button = new wxButton(this, ID_UPGRADE, _BUTTON_("upgrade package"));
	wxButton* remove_button  = new wxButton(this, ID_REMOVE,  _BUTTON_("remove package"));
/*	
	description_window = new HtmlWindowToBrowser(this, wxID_ANY, wxDefaultPosition, wxSize(540,100), wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER);
	setDefaultPackageStatus();
	
	(install_button = new wxButton(this, ID_INSTALL, _MENU_("install package")))->Disable();
	(upgrade_button = new wxButton(this, ID_UPGRADE, _MENU_("upgrade package")))->Disable();
	(remove_button  = new wxButton(this, ID_REMOVE,  _MENU_("remove package")))->Disable();
	(cancel_button  = new wxButton(this, ID_CANCEL,  _MENU_("cancel changes")))->Disable();

	apply_button = new wxButton(this, ID_APPLY,   _MENU_("apply changes"));
	
	// Init sizer
	
	h2->AddStretchSpacer();
	h2->Add(install_button);
	h2->AddStretchSpacer();
	h2->Add(upgrade_button);
	h2->AddStretchSpacer();
	h2->Add(remove_button);
	h2->AddStretchSpacer();
	h2->Add(cancel_button);
	h2->AddStretchSpacer();
*/
/*	wxBoxSizer *h2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *h3 = new wxBoxSizer(wxHORIZONTAL);
	
	h3->AddStretchSpacer();
	h3->Add(apply_button);
	h3->AddStretchSpacer();
//	v->Add(h2);
//	v->Add(h3);*/
	
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
	//(new WelcomeWindow)->Show();
}

/*
void PackagesWindow::onUpdateCheckFinished(wxCommandEvent&) {
	setDefaultPackageStatus();
}*/

void PackagesWindow::onPackageSelect(wxCommandEvent& ev) {
//	updateButtons(package_list->getSelection());
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

/*
void PackagesWindow::onActionChange(wxCommandEvent& ev) {
	PackageVersionDataP pack = package_list->getSelection();
	if (!pack) return;
	PackageAction& action = package_data[pack].second;
	switch (ev.GetId()) {
		case ID_INSTALL:
			action = ACTION_INSTALL;
			SelectPackageDependencies(pack);
			break;
		case ID_REMOVE: 
			action = ACTION_UNINSTALL;
			RemovePackageDependencies(pack);
			break;
		case ID_UPGRADE:
			action = ACTION_UPGRADE;
			SelectPackageDependencies(pack);
			break;
		case ID_CANCEL:
			switch (package_data[pack].first) {
				case STATUS_INSTALLED:
					SelectPackageDependencies(pack);
					break;
				case STATUS_NOT_INSTALLED:
					RemovePackageDependencies(pack);
					break;
				case STATUS_UPGRADEABLE:
					if (action == ACTION_UPGRADE)
						DowngradePackageDependencies(pack);
					else
						SelectPackageDependencies(pack);
					break;
			}
			action = (pack->app_version > file_version) ? ACTION_NEW_MSE : ACTION_NOTHING;
			break;
	}
	updateButtons(package_list->getSelection());
	package_list->Refresh();
}

void PackagesWindow::onApplyChanges(wxCommandEvent& ev) {
	list<PackageVersionDataP> to_install, to_remove;

	FOR_EACH(pack, update_version_data->packages) {
		switch (package_data[pack].second) {
			case ACTION_INSTALL:
				to_install.push_back(pack);
				break;
			case ACTION_UPGRADE:
				to_install.push_back(pack);
			case ACTION_UNINSTALL:
				to_remove.push_back(pack);
			default:;
		}
	}

	FOR_EACH(pack, to_remove) {
		String filename = packages.openAny(pack->name, true)->absoluteFilename();
		if (!remove_file_or_dir(filename)) {
			handle_error(_("Cannot delete ") + filename + _(" to remove package ") + pack->name + _(". ")
				_("Other packages may have been removed, including packages that this on is dependent on. Please remove manually."));
		}
	}

	FOR_EACH(pack, to_install) {
		wxURL url(pack->url);
		wxInputStream* is = url.GetInputStream();
		if (!is) {
			handle_error(_("Cannot fetch file ") + pack->url + _(" to install package ") + pack->name + _(". ")
				_("Other packages may have been installed, including packages that depend on this one. ")
				_("Please remove those packages manually or install this one manually."));
		}
		wxString filename = wxFileName::CreateTempFileName(_(""));
		wxFileOutputStream os (filename);

		os.Write(*is);
		os.Close();

		InstallerP inst(new Installer);
		inst->open(filename);
		inst->install(isInstallLocal(settings.install_type), false);

		delete is;
		wxRemoveFile(filename);
	}

	setDefaultPackageStatus();
	updateButtons(package_list->getSelection());
	package_list->Refresh();

	handle_pending_errors();

	packages.clearPackageCache();
}

void PackagesWindow::updateButtons(const PackageVersionDataP& pack) {

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
	} else /* status == STATUS_UPGRADEABLE * / {
		install_button->Disable();
		remove_button->Enable(action != ACTION_UNINSTALL);
		upgrade_button->Enable(action != ACTION_UPGRADE);
		cancel_button->Enable(action != ACTION_NOTHING);
	}
}

void PackagesWindow::setDefaultPackageStatus() {
	if (!update_version_data) return;
	FOR_EACH(p, update_version_data->packages) {
		PackagedP pack;
		try { pack = packages.openAny(p->name, true); }
		catch (PackageNotFoundError&) { } // We couldn't open a package... no cause for alarm
		
		if (!pack || !(wxFileExists(pack->absoluteFilename()) || wxDirExists(pack->absoluteFilename()))) {
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

/// Select the dependencies for a package
/**
 *  \param pack The package data to check dependencies for.
 *
 *  This function will select all the dependencies for the package, to ensure
 *  that the user can't install a package without it's dependencies.
 * /
void PackagesWindow::SelectPackageDependencies (PackageVersionDataP pack) {
	FOR_EACH(dep, pack->depends) {
		if (packages.checkDependency(*dep, false)) //It's already installed.
			continue;
		FOR_EACH(p, update_version_data->packages) {
			if (p->name == dep->package) { //We have a match.
				if (p->version >= dep->version) { //Versions line up

					PackageStatus& status = package_data[p].first;
					PackageAction& action = package_data[p].second;
					if (status == STATUS_NOT_INSTALLED)
						action = ACTION_INSTALL;
					else if (status == STATUS_UPGRADEABLE)
						action = ACTION_UPGRADE;
					else //status == STATUS_INSTALLED
						action = ACTION_NOTHING;
					break;
				}
			}
		}
		// TODO: Decide what to do if a dependency can't be met.
		//       It shouldn't happen with a decently maintained updates list.
		//       But it could, and we need to decide what to do in this situation.
	}
}

/// This finds all packages that depend on the one provided and marks them for removal.
void PackagesWindow::RemovePackageDependencies (PackageVersionDataP pack) {
	FOR_EACH(p, update_version_data->packages) {
		FOR_EACH(dep, p->depends) {
			if (pack->name == dep->package) {
				PackageStatus& status = package_data[p].first;
				PackageAction& action = package_data[p].second;

				if (status != STATUS_NOT_INSTALLED)
					action = ACTION_UNINSTALL;
				else // status == STATUS_NOT_INSTALLED
					action = p->app_version > file_version ? ACTION_NEW_MSE : ACTION_NOTHING;
				break;
			}
		}
	}
}

/// This deals with the complexities of a downgrade and its dependencies.
void PackagesWindow::DowngradePackageDependencies (PackageVersionDataP pack) {
	PackagedP old_pack = packages.openAny(pack->name, true);
	FOR_EACH(dep, old_pack->dependencies) {
		// dependencies the old version has, but the new one might not.
		if (packages.checkDependency(*dep, false)) //It's already installed.
			continue;
		FOR_EACH(p, update_version_data->packages) {
			if (p->name == dep->package) { //We have a match.
				if (p->version >= dep->version) { //Versions line up
					if (p->app_version > file_version) //We can't install this
						continue;

					PackageStatus& status = package_data[p].first;
					PackageAction& action = package_data[p].second;
					if (status == STATUS_NOT_INSTALLED)
						action = ACTION_INSTALL;
					else if (status == STATUS_UPGRADEABLE)
						action = ACTION_UPGRADE;
					break;
				}
			}
		}
		// TODO: Decide what to do if a dependency can't be met.
		//       It shouldn't happen with a decently maintained updates list.
		//       But it could, and we need to decide what to do in this situation.
		//       Ideally, some sort of error should occur, such that we don't have packages
		//       with unmet dependencies.
	}

	FOR_EACH(p, update_version_data->packages) {
		// dependencies that can no longer be met.
		FOR_EACH(dep, p->depends) {
			if (pack->name == dep->package) {
				if (old_pack->version > dep->version) {
					PackageStatus& status = package_data[p].first;
					PackageAction& action = package_data[p].second;

					if (status != STATUS_NOT_INSTALLED)
						action = ACTION_UNINSTALL;
					else // status == STATUS_NOT_INSTALLED
						action = p->app_version > file_version ? ACTION_NEW_MSE : ACTION_NOTHING;
					break;
				}
			}
		}
	}
}
*/

BEGIN_EVENT_TABLE(PackagesWindow, wxDialog)
//	EVT_COMMAND(wxID_ANY, UPDATE_CHECK_FINISHED_EVT, PackagesWindow::onUpdateCheckFinished)
	EVT_LISTBOX(ID_PACKAGE_LIST, PackagesWindow::onPackageSelect)
	EVT_BUTTON(ID_INSTALL,  PackagesWindow::onActionChange)
	EVT_BUTTON(ID_REMOVE,   PackagesWindow::onActionChange)
	EVT_BUTTON(ID_UPGRADE,  PackagesWindow::onActionChange)
	EVT_BUTTON(wxID_OK,     PackagesWindow::onOk)
	EVT_UPDATE_UI(wxID_ANY, PackagesWindow::onUpdateUI)
END_EVENT_TABLE()
