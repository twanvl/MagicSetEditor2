//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <data/settings.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : Single card export

void export_image(const SetP& set, const CardP& card, const String& filename) {
	Image img = export_bitmap(set, card).ConvertToImage();
	img.SaveFile(filename);	// can't use Bitmap::saveFile, it wants to know the file type
							// but image.saveFile determines it automagicly
}

class UnzoomedDataViewer : public DataViewer {
  public:
	UnzoomedDataViewer(bool use_zoom_settings)
		: use_zoom_settings(use_zoom_settings)
	{}
	virtual Rotation getRotation() const;
  private:
	bool use_zoom_settings;
};
Rotation UnzoomedDataViewer::getRotation() const {
	if (use_zoom_settings) {
		return DataViewer::getRotation();
	} else {
		if (!stylesheet) stylesheet = set->stylesheet;
		return Rotation(0, stylesheet->getCardRect(), 1.0, 1.0, ROTATION_ATTACH_TOP_LEFT);
	}
}

Bitmap export_bitmap(const SetP& set, const CardP& card) {
	if (!set) throw Error(_("no set"));
	// create viewer
	UnzoomedDataViewer viewer(!settings.stylesheetSettingsFor(set->stylesheetFor(card)).card_normal_export());
	viewer.setSet(set);
	viewer.setCard(card);
	// size of cards
	RealSize size = viewer.getRotation().getExternalSize();
	// create bitmap & dc
	Bitmap bitmap((int) size.width, (int) size.height);
	if (!bitmap.Ok()) throw InternalError(_("Unable to create bitmap"));
	wxMemoryDC dc;
	dc.SelectObject(bitmap);
	// draw
	viewer.draw(dc);
	dc.SelectObject(wxNullBitmap);
	return bitmap;
}

// ----------------------------------------------------------------------------- : Multiple card export
