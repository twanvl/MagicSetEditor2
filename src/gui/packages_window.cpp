//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/packages_window.hpp>
#include <gui/control/tree_list.hpp>
#include <gui/thumbnail_thread.hpp>
#include <gui/util.hpp>
#include <util/io/package_manager.hpp>
#include <util/window_id.hpp>
#include <data/installer.hpp>
#include <data/settings.hpp>
#include <gfx/gfx.hpp>
#include <boost/scoped_ptr.hpp>
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
	~PackageUpdateList() {
		thumbnail_thread.abort(this);
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
  public:
	class TreeItem;
	typedef intrusive_ptr<TreeItem> TreeItemP;
	class TreeItem : public Item {
	  public:
		TreeItem() : position_type(TYPE_OTHER), position_hint(1000000) {}
		String label;
		vector<TreeItemP> children;
		InstallablePackageP package;
		Bitmap icon, icon_grey;
		// positioning
		enum PackageType {
			TYPE_PROG,
			TYPE_LOCALE,
			TYPE_GAME,
			TYPE_STYLESHEET,
			TYPE_EXPORT_TEMPLATE,
			TYPE_SYMBOL_FONT,
			TYPE_INCLUDE,
			TYPE_FONT,
			TYPE_OTHER,
		}   position_type;
		int position_hint;
		
		void add(const InstallablePackageP& package, const String& path, int level = -1);
		void toItems(vector<TreeList::ItemP>& items);
		void setIcon(const Image& image);
		bool highlight() const;
		
		static PackageType package_type(const PackageDescription& desc);
	};
};

// ----------------------------------------------------------------------------- : PackageUpdateList::TreeItem

DECLARE_TYPEOF_COLLECTION(PackageUpdateList::TreeItemP);


void PackageUpdateList::TreeItem::add(const InstallablePackageP& package, const String& path, int level) {
	// this node
	this->level = level;
	PackageType new_type = package_type(*package->description);
	int new_hint = package->description->position_hint;
	if (new_type < position_type || (new_type == position_type && new_hint < position_hint)) {
		// this is a lower position hint, use it
		position_type = new_type;
		position_hint = new_hint;
	}
	// end of the path?
	if (path.empty()) {
		assert(!this->package);
		this->package = package;
		return;
	}
	// split path
	size_t pos = path.find_first_of(_('/'));
	String name = path.substr(0,pos);
	String rest = pos == String::npos ? _("") : path.substr(pos+1);
	// find/add child
	FOR_EACH(ti, children) {
		if (ti->label == name) {
			// already have this child
			if (pos == String::npos && ti->package) {
				// two packages with the same path
				TreeItemP ti2(new TreeItem);
				ti2->label = name;
				children.insert(ti_IT.first, ti2);
				ti2->add(package, rest, level + 1);
			} else {
				ti->add(package, rest, level + 1);
			}
			return;
		}
	}
	// don't have this child
	TreeItemP ti(new TreeItem);
	children.push_back(ti);
	ti->label = name;
	ti->add(package, rest, level + 1);
}

bool compare_pos_hint(const PackageUpdateList::TreeItemP& a, const PackageUpdateList::TreeItemP& b) {
	if (a->position_type < b->position_type) return true;
	if (a->position_type > b->position_type) return false;
	if (a->position_hint < b->position_hint) return true;
	if (a->position_hint > b->position_hint) return false;
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
	if (package && package->willBeInstalled()) return true;
	FOR_EACH_CONST(c,children) if (c->highlight()) return true;
	return false;
}

PackageUpdateList::TreeItem::PackageType PackageUpdateList::TreeItem::package_type(const PackageDescription& desc) {
	if (desc.name == mse_package)                           return TYPE_PROG;
	size_t pos = desc.name.find_last_of(_('.'));
	if (pos == String::npos)                                return TYPE_OTHER;
	if (is_substr(desc.name,pos,_(".mse-locale")))          return TYPE_LOCALE;
	if (is_substr(desc.name,pos,_(".mse-game")))            return TYPE_GAME;
	if (is_substr(desc.name,pos,_(".mse-style")))           return TYPE_STYLESHEET;
	if (is_substr(desc.name,pos,_(".mse-export-template"))) return TYPE_EXPORT_TEMPLATE;
	if (is_substr(desc.name,pos,_(".mse-symbol-font")))     return TYPE_SYMBOL_FONT;
	if (is_substr(desc.name,pos,_(".mse-include")))         return TYPE_INCLUDE;
	if (is_substr(desc.name,pos,_(".ttf")))                 return TYPE_FONT;
	return TYPE_OTHER;
}

void PackageUpdateList::TreeItem::setIcon(const Image& img) {
	Image image = img;
	int iw = image.GetWidth(), ih = image.GetHeight();
	if (ih > 107) {
		int w = 107 * iw / ih;
		image = resample(image, w, 107);
	} else if (iw > 107) {
		int h = 107 * ih / iw;
		image = resample(image, 107, h);
	}
	if (package) package->description->icon = image;
	Image resampled = resample_preserve_aspect(image,16,16);
	icon = Bitmap(resampled);
	saturate(resampled, -.75);
	set_alpha(resampled,0.5);
	icon_grey = Bitmap(resampled);
}

// ----------------------------------------------------------------------------- : PackageIconRequest

/// wx doesn't allow seeking on InputStreams from a wxURL
/// The built in buffer class is too stupid to seek, so we must do it ourselfs
class SeekAtStartInputStream : public wxFilterInputStream {
  public:
	SeekAtStartInputStream(wxInputStream& stream)
		: wxFilterInputStream(stream)
		, buffer_pos(0)
	{
		m_parent_i_stream->Read(buffer, 1024);
		buffer_size = m_parent_i_stream->LastRead();
	}
	
	bool IsSeekable() const { return true; }
  protected:
    virtual size_t OnSysRead(void *buffer, size_t bufsize) {
		size_t len = min(buffer_size - buffer_pos, bufsize);
		memcpy(buffer, this->buffer + buffer_pos, len);
		buffer_pos += len;
		m_parent_i_stream->Read((Byte*)buffer + len, bufsize - len);
		return m_parent_i_stream->LastRead() + len; 
    }
    virtual wxFileOffset OnSysSeek(wxFileOffset seek, wxSeekMode mode) {
		if      (mode == wxFromStart)   buffer_pos = seek;
		else if (mode == wxFromCurrent) buffer_pos += seek;
		else                            assert(false);
		assert(buffer_pos < buffer_size);
		return buffer_pos;
    }
    virtual wxFileOffset OnSysTell() const {
		assert(buffer_pos < buffer_size);
		return buffer_pos;
    }
  private:
	size_t buffer_size, buffer_pos;
	Byte buffer[1024];
};

class PackageIconRequest : public ThumbnailRequest {
  public:
	PackageIconRequest(PackageUpdateList* list, PackageUpdateList::TreeItem* ti)
		: ThumbnailRequest(
			list,
			_("package_") + ti->package->description->icon_url + _("_") + ti->package->description->version.toString(),
			wxDateTime(1,wxDateTime::Jan,2000))
		, list(list), ti(ti)
	{}
	
	virtual Image generate() {
		wxURL url(ti->package->description->icon_url);
		scoped_ptr<wxInputStream> isP(url.GetInputStream());
		if (!isP) return wxImage();
		SeekAtStartInputStream is2(*isP);
		Image result(is2);
		return result;
	}
	virtual void store(const Image& image) {
		if (!image.Ok()) return;
		ti->setIcon(image);
		list->Refresh(false);
	}
  private:
	PackageUpdateList* list;
	PackageUpdateList::TreeItem* ti;
};

// ----------------------------------------------------------------------------- : PackageUpdateList : implementation

void PackageUpdateList::initItems() {
	// packages to tree
	TreeItem root;
	FOR_EACH(ip, parent->installable_packages) {
		String group = ip->description->installer_group;
		if (group.empty()) group = _("custom");
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
			ti.setIcon(p->description->icon);
		} else if (p) {
			ti.setIcon(load_resource_image(_("installer_package")));
			if (!p->description->icon_url.empty()) {
				// download icon
				thumbnail_thread.request(new_intrusive2<PackageIconRequest>(this,&ti));
			}
		} else if (ti.position_type == TreeItem::TYPE_LOCALE) {
			ti.setIcon(load_resource_image(_("installer_locales")));
		} else {
			ti.setIcon(load_resource_image(_("installer_group")));
		}
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
		dc.DrawText(capitalize_sentence(ti.label), x+18, y+2);
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
			if (ti.package->status & PACKAGE_INSTALLED) {
				dc.SetTextForeground(lerp(color,Color(0,0,255),0.5));
				dc.DrawText(_LABEL_("upgrade package"), x+1,y+2);
			} else {
				dc.SetTextForeground(lerp(color,Color(0,255,0),0.5));
				dc.DrawText(_LABEL_("install package"), x+1,y+2);
			}
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
