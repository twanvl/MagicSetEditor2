//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>

// ----------------------------------------------------------------------------- : MSE2FileFormat

/// The file format of MSE2 files
class MSE2FileFormat : public FileFormat {
  public:
	virtual String extension()          { return _("mse-set"); }
	virtual String name()               { return _("Magic Set Editor sets (*.mse-set)"); }
	virtual bool canImport()            { return true; }
	virtual bool canExport(const Game&) { return true; }
	virtual SetP importSet(const String& filename) {
		settings.addRecentFile(filename);
		SetP set(new Set);
		set->open(filename);
		return set;
	}
	virtual void exportSet(Set& set, const String& filename) {
		settings.addRecentFile(filename);
		set.saveAs(filename);
		set.actions.setSavePoint();;
	}
};

FileFormatP mse2_file_format() {
	return new_shared<MSE2FileFormat>();
}
