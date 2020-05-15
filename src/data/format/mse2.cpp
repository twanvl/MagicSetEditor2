//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>

// ----------------------------------------------------------------------------- : MSE2FileFormat

/// The file format of MSE2 files
class MSE2FileFormat : public FileFormat {
public:
  String extension() override          { return _("mse-set"); }
  String matches() override            { return _("*.mse-set;set"); }
  String name() override               { return _("Magic Set Editor sets (*.mse-set)"); }
  bool canImport() override            { return true; }
  bool canExport(const Game&) override { return true; }
  SetP importSet(const String& filename) override {
    wxString set_name = filename;
    // Strip "/set" or "/set.mset-set" from the end, this allows opening directories as set files
    if (filename.EndsWith(_(".mse-set/set")) || filename.EndsWith(_(".mse-set\\set"))) {
      set_name = filename.substr(0, filename.size() - 4);
    } else if (filename.EndsWith(_(".mse-set/set.mse-set")) || filename.EndsWith(_(".mse-set\\set.mse-set"))) {
      set_name = filename.substr(0, filename.size() - 12);
    }
    SetP set = make_intrusive<Set>();
    set->open(set_name);
    settings.addRecentFile(set_name);
    return set;
  }
  void exportSet(Set& set, const String& filename, bool is_copy) override {
    if (is_copy) {
      set.saveCopy(filename);
    } else {
      set.saveAs(filename);
      settings.addRecentFile(filename);
      set.actions.setSavePoint();
    }
  }
};

FileFormatP mse2_file_format() {
  return make_unique<MSE2FileFormat>();
}
