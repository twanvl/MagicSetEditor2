//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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
	virtual String extension()          { return _("mse-set"); }
	virtual String matches()            { return _("*.mse-set;set"); }
	virtual String name()               { return _("Magic Set Editor sets (*.mse-set)"); }
	virtual bool canImport()            { return true; }
	virtual bool canExport(const Game&) { return true; }
	virtual SetP importSet(const String& filename) {
		wxString set_name = filename;
		// Strip "/set" from the end, newer wx versions have a function for this:
		//   filename.EndsWith(_("/set"), &set_name);
		if (filename.size() > 4 && filename.substr(filename.size()-4) == _("/set")) {
			set_name = filename.substr(0, filename.size()-4);
		}
		SetP set(new Set);
		set->open(set_name);
		settings.addRecentFile(set_name);
		return set;
	}
	virtual void exportSet(Set& set, const String& filename, bool is_copy) {
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
	return intrusive(new MSE2FileFormat());
}
