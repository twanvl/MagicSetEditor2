//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/package_list.hpp>
#include <util/io/package_manager.hpp>
#include <util/alignment.hpp>

// ----------------------------------------------------------------------------- : PackageList

PackageList::PackageList(Window* parent, int id, int direction)
	: GalleryList(parent, id, direction)
{
	item_size = wxSize(108, 150);
	SetThemeEnabled(true);
}

size_t PackageList::itemCount() const {
	return packages.size();
}

void PackageList::drawItem(DC& dc, int x, int y, size_t item, bool selected) {
	PackageData& d = packages.at(item);
	RealRect rect(RealPoint(x,y),item_size);
	RealPoint pos;
	int w, h;
	// draw image
	if (d.image.Ok()) {
		dc.DrawBitmap(d.image, x + int(align_delta_x(ALIGN_CENTER, item_size.x, d.image.GetWidth())), y + 3, true);
	}
	// draw short name
	dc.SetFont(wxFont(12,wxSWISS,wxNORMAL,wxBOLD,false,_("Arial")));
	dc.GetTextExtent(capitalize(d.package->short_name), &w, &h);
	pos = align_in_rect(ALIGN_CENTER, RealSize(w,h), rect);
	dc.DrawText(capitalize(d.package->short_name), (int)pos.x, (int)pos.y + 110);
	// draw name
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	dc.GetTextExtent(d.package->full_name, &w, &h);
	RealPoint text_pos = align_in_rect(ALIGN_CENTER, RealSize(w,h), rect);
	dc.DrawText(d.package->full_name, (int)text_pos.x, (int)text_pos.y + 130);
}

struct PackageList::ComparePackagePosHint {
	bool operator () (const PackageData& a, const PackageData& b) {
		// use position_hints to determine order
		if (a.package->position_hint < b.package->position_hint) return true;
		if (a.package->position_hint > b.package->position_hint) return false;
		// ensure a deterministic order: use the names
		return a.package->name() < b.package->name();
	}
};

void PackageList::showData(const String& pattern) {
	// clear
	packages.clear();
	// find matching packages
	String f = ::packages.findFirst(pattern);
	while (!f.empty()) {
		// try to open the package
//		try {
			PackagedP package = ::packages.openAny(f, true);
			// open image
			InputStreamP stream = package->openIconFile();
			Image img;
			Bitmap bmp;
			if (stream && img.LoadFile(*stream)) {
				bmp = Bitmap(img);
			}
			// add to list
			packages.push_back(PackageData(package, bmp));
/*		}
		// If there are errors we don't add the package to the list
		catch (Error e) {
			handleError(e, false);
		} catch (std::exception e) {
			// we don't throw Exceptions ourselfs, so this is probably something serious
			handleError(InternalError(String(csconv_(e.what()))), false);
		} catch (...) {
			handleError(InternalError(_("An unexpected exception occurred, \nplease save your work (use save as to so you don't overwrite things).\n And restart Magic Set Editor")), false);
		}*/
		// Next package
		f = wxFindNextFile();
	}
	// sort list
	sort(packages.begin(), packages.end(), ComparePackagePosHint());
	// update list
	update();
}

void PackageList::clear() {
	packages.clear();
	update();
}

void PackageList::select(const String& name, bool send_event) {
	for (vector<PackageData>::const_iterator it = packages.begin() ; it != packages.end() ; ++it) {
		if (it->package->name() == name) {
			selection = it - packages.begin();
			update();
			if (send_event) {
				sendEvent(EVENT_GALLERY_SELECT);
			}
			return;
		}
	}
	selection = NO_SELECTION;
	update();
	return;
}
