//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	item_size = wxSize(110, 150);
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
		dc.DrawBitmap(d.image, x + align_delta_x(ALIGN_CENTER, item_size.width, d.image.GetWidth()), y + 3);
	}
	// draw short name
	dc.SetFont(wxFont(12,wxSWISS,wxNORMAL,wxBOLD,false,_("Arial")));
	dc.GetTextExtent(capitalize(d.package->name()), &w, &h);
	pos = align_in_rect(ALIGN_CENTER, RealSize(w,h), rect);
	dc.DrawText(capitalize(d.package->name()), pos.x, pos.y + 110);
	// draw name
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	dc.GetTextExtent(d.package->fullName(), &w, &h);
	RealPoint text_pos = align_in_rect(ALIGN_CENTER, RealSize(w,h), rect);
	dc.DrawText(d.package->fullName(), text_pos.x, text_pos.y + 130);
}

void PackageList::showData(const String& pattern) {
	// clear
	packages.clear();
	// find matching packages
	String f = ::packages.findFirst(pattern);
	while (!f.empty()) {
		// try to open the package
//		try {
			PackageP package = ::packages.openAny(f);
			// open image
			InputStreamP stream = package->openIconFile();
			Bitmap bmp;
			if (stream) {
				bmp = Bitmap(Image(*stream));
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
