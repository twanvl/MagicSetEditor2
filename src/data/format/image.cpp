//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/tagged_string.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>
#include <data/settings.hpp>
#include <render/card/viewer.hpp>
#include <wx/filename.h>

// ----------------------------------------------------------------------------- : Single card export

void export_image(const SetP& set, const CardP& card, const String& filename) {
  Image img = export_bitmap(set, card).ConvertToImage();
  img.SaveFile(filename);  // can't use Bitmap::saveFile, it wants to know the file type
              // but image.saveFile determines it automagicly
}

class ZoomOverrideDataViewer : public DataViewer {
public:
  ZoomOverrideDataViewer(bool use_zoom_settings, double zoom = 1.0)
    : use_zoom_settings(use_zoom_settings), zoom(zoom)
  {}
  Rotation getRotation() const override;
private:
  bool use_zoom_settings;
  double zoom  = 1.0;
  double angle = 0.0;
};
Rotation ZoomOverrideDataViewer::getRotation() const {
  Rotation rot;
  if (use_zoom_settings) {
    rot =  DataViewer::getRotation();
    rot.setZoom(rot.getZoom() * zoom);
  }
  else {
    if (!stylesheet) stylesheet = set->stylesheet;
    rot = Rotation(angle, stylesheet->getCardRect(), zoom, 1.0, ROTATION_ATTACH_TOP_LEFT);
  }
  return rot;
}

Bitmap export_bitmap(const SetP& set, const CardP& card, double zoom /*= 1.0*/) {
  if (!set) throw Error(_("no set"));
  // create viewer
  ZoomOverrideDataViewer viewer(!settings.stylesheetSettingsFor(set->stylesheetFor(card)).card_normal_export(), zoom);
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


void export_images(const SetP& set, const vector<CardP>& cards,
                   const String& path, const String& filename_template, FilenameConflicts conflicts)
{
  wxBusyCursor busy;
  // Script
  ScriptP filename_script = parse(filename_template, nullptr, true);
  // Path
  wxFileName fn(path);
  // Export
  std::set<String> used; // for CONFLICT_NUMBER_OVERWRITE
  FOR_EACH_CONST(card, cards) {
    // filename for this card
    Context& ctx = set->getContext(card);
    String filename = clean_filename(untag(ctx.eval(*filename_script)->toString()));
    if (!filename) continue; // no filename -> no saving
    // full path
    fn.SetFullName(filename);
    // does the file exist?
    if (!resolve_filename_conflicts(fn, conflicts, used)) continue;
    // write image
    filename = fn.GetFullPath();
    used.insert(filename);
    export_image(set, card, filename);
  }
}
