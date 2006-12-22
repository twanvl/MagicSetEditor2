//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : Single card export

void export_image(const SetP& set, const CardP& card, const String& filename) {
	Image img = export_bitmap(set, card).ConvertToImage();
	img.SaveFile(filename);	// can't use Bitmap::saveFile, it wants to know the file type
							// but image.saveFile determines it automagicly
}

Bitmap export_bitmap(const SetP& set, const CardP& card) {
	// create viewer
	DataViewer viewer;
	viewer.setSet(set);
	viewer.setCard(card);
	// style to use
	StyleSheetP style = set->stylesheetFor(card);
	// size of cards
	if (settings.stylesheetSettingsFor(*style).card_normal_export()) {
		// TODO
//		viewer.rotation.angle = 0;
//		viewer.rotation.zoom  = 1.0;
	}
	RealSize size = viewer.getRotation().getExternalSize();
	// create bitmap & dc
	Bitmap bitmap(size.width, size.height);
	if (!bitmap.Ok()) throw InternalError(_("Unable to create bitmap"));
	wxMemoryDC dc;
	dc.SelectObject(bitmap);
	// draw
	viewer.draw(dc);
	dc.SelectObject(wxNullBitmap);
	return bitmap;
}

// ----------------------------------------------------------------------------- : Multiple card export
