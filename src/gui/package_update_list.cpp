//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/package_update_list.hpp>
#include <gui/thumbnail_thread.hpp>
#include <gui/util.hpp>
#include <gfx/gfx.hpp>
#include <boost/scoped_ptr.hpp>
#include <wx/url.h>

DECLARE_TYPEOF_COLLECTION(InstallablePackageP);
DECLARE_TYPEOF_COLLECTION(PackageUpdateList::TreeItemP);
DECLARE_TYPEOF_COLLECTION(TreeList::ItemP);


// ----------------------------------------------------------------------------- : PackageUpdateList::TreeItem

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

/// Retrieve the icon for a package
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

PackageUpdateList::PackageUpdateList(Window* parent, const InstallablePackages& packages, bool show_only_installable, int id)
	: TreeList(parent, id)
	, packages(packages)
	, show_only_installable(show_only_installable)
{
	item_height = max(item_height,19);
	rebuild();
}
PackageUpdateList::~PackageUpdateList() {
	thumbnail_thread.abort(this);
}

void PackageUpdateList::initItems() {
	// add packages to tree
	TreeItem root;
	FOR_EACH_CONST(ip, packages) {
		String group = ip->description->installer_group;
		if (group.empty()) group = _("custom");
		if (!show_only_installable || ip->installer) {
			root.add(ip, group);
		}
	}
	// tree to treelist items
	items.clear();
	root.toItems(items);
	// init image list
	FOR_EACH(i,items) {
		TreeItem& ti = static_cast<TreeItem&>(*i);
		const InstallablePackageP& p = ti.package;
		// load icon
		Image image;
		if (p && p->description->icon.Ok()) { // it has an icon
			ti.setIcon(p->description->icon);
		} else if (p) {                       // it doesn't have an icon (yet)
			ti.setIcon(load_resource_image(_("installer_package")));
			if (!p->description->icon_url.empty()) {
				// download icon
				thumbnail_thread.request(intrusive(new PackageIconRequest(this,&ti)));
			}
		} else if (ti.position_type == TreeItem::TYPE_LOCALE) { // locale folder
			ti.setIcon(load_resource_image(_("installer_locales")));
		} else {                                                // other folder
			ti.setIcon(load_resource_image(_("installer_group")));
		}
	}
}

void PackageUpdateList::drawItem(DC& dc, size_t index, size_t column, int x, int y, bool selected) const {
	// offset for drawing
	x += 1;  y += 3;
	// the item
	const TreeItem& ti = static_cast<const TreeItem&>(*items[index]);
	Color color = wxSystemSettings::GetColour(selected ? wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT);
	if (column == 0) {
		// Name
		const Bitmap& bmp = ti.highlight() ? ti.icon : ti.icon_grey;
		if (bmp.Ok()) dc.DrawBitmap(bmp, x-1,y-2);
		dc.SetTextForeground(color);
		dc.DrawText(capitalize_sentence(ti.label), x+17, y);
	} else if (column == 1 && ti.package) {
		// Status
		InstallablePackage& package = *ti.package;
		if (package.has(PACKAGE_CONFLICTS)) {
			dc.SetTextForeground(lerp(color,Color(255,0,0),0.8));
			dc.DrawText(_LABEL_("package conflicts"), x,y);
		} else if (package.has(PACKAGE_MODIFIED)) {
			dc.SetTextForeground(lerp(color,Color(255,255,0),0.5));
			dc.DrawText(_LABEL_("package modified"), x,y);
		} else if (package.has(PACKAGE_UPDATES)) {
			dc.SetTextForeground(lerp(color,Color(0,0,255),0.5));
			dc.DrawText(_LABEL_("package updates"), x,y);
		} else if (package.has(PACKAGE_INSTALLED)) {
			dc.SetTextForeground(color);
			dc.DrawText(_LABEL_("package installed"), x,y);
		} else if (package.has(PACKAGE_INSTALLABLE)) {
			dc.SetTextForeground(lerp(color,Color(128,128,128),0.6));
			dc.SetTextForeground(color);
			dc.DrawText(_LABEL_("package installable"), x,y);
		}
	} else if (column == 2 && ti.package) {
		// Action
		InstallablePackage& package = *ti.package;
		if (package.has(PACKAGE_ACT_INSTALL)) {
			if (package.has(PACKAGE_UPDATES)) {
				dc.SetTextForeground(lerp(color,Color(0,0,255),0.6));
				dc.DrawText(_LABEL_("upgrade package"), x,y);
			} else if (package.has(PACKAGE_INSTALLED)) {
				dc.SetTextForeground(lerp(color,Color(0,0,255),0.2));
				dc.DrawText(_LABEL_("reinstall package"), x,y);
			} else {
				dc.SetTextForeground(lerp(color,Color(0,255,0),0.6));
				dc.DrawText(_LABEL_("install package"), x,y);
			}
		} else if (package.has(PACKAGE_ACT_REMOVE)) {
			dc.SetTextForeground(lerp(color,Color(255,0,0),0.7));
			dc.DrawText(_LABEL_("remove package"), x,y);
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
		return cs.x - 240;
	} else {
		return 120;
	}
}

