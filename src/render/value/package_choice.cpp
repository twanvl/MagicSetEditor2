//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/package_choice.hpp>
#include <util/io/package_manager.hpp>

DECLARE_TYPEOF_COLLECTION(PackagedP);
DECLARE_TYPEOF_COLLECTION(PackageChoiceValueViewer::Item);

// ----------------------------------------------------------------------------- : PackageChoiceValueViewer

IMPLEMENT_VALUE_VIEWER(PackageChoice);

struct PackageChoiceValueViewer::ComparePackagePosHint {
	bool operator () (const PackagedP& a, const PackagedP& b) {
		// use position_hints to determine order
		if (a->position_hint < b->position_hint) return true;
		if (a->position_hint > b->position_hint) return false;
		// ensure a deterministic order: use the names
		return a->name() < b->name();
	}
};

void PackageChoiceValueViewer::initItems() {
	vector<PackagedP> choices;
	package_manager.findMatching(field().match, choices);
	sort(choices.begin(), choices.end(), ComparePackagePosHint());
	FOR_EACH(p, choices) {
		Item i;
		i.package_name = p->relativeFilename();
		i.name = capitalize_sentence(p->short_name);
		Image image;
		InputStreamP stream = p->openIconFile();
		if (stream && image.LoadFile(*stream)) {
			i.image = Bitmap(resample(image, 16,16));
		}
		items.push_back(i);
	}
}

void PackageChoiceValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	// find item
	String text = value().package_name;
	Bitmap image;
	if (value().package_name.empty()) {
		text = field().empty_name;
	} else {
		FOR_EACH(i, items) {
			if (i.package_name == value().package_name) {
				text = i.name;
				image = i.image;
			}
		}
	}
	// draw image
	if (image.Ok()) {
		dc.DrawBitmap(image, RealPoint(0,0));
	}
	// draw text
	dc.SetFont(style().font, 1.0);
	RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, RealSize(0, dc.GetCharHeight()), dc.getInternalRect()) + RealSize(17., 0);
	dc.DrawTextWithShadow(text, style().font, pos);
}
