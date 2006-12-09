//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/formats.hpp>

// ----------------------------------------------------------------------------- : MtgEditorFileFormat

/// The file format of Mtg Editor files
class MtgEditorFileFormat : public FileFormat {
  public:
	virtual String extension()          { return _("set"); }
	virtual String name()               { return _("Mtg Editor files (*.set)"); }
	virtual bool canImport()            { return true; }
	virtual bool canExport(const Game&) { return false; }
	virtual SetP importSet(const String& filename);
};

FileFormatP mtg_editor_file_format() {
	return new_shared<MtgEditorFileFormat>();
}

// ----------------------------------------------------------------------------- : Importing

SetP MtgEditorFileFormat::importSet(const String& filename) {
	return SetP();//TODO
}
