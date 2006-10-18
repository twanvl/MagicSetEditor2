//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/package_manager.hpp>
#include <util/error.hpp>
#include <data/game.hpp>

// ----------------------------------------------------------------------------- : PackageManager

String program_dir() {
	return wxGetCwd(); //TODO
}

PackageManager packages;


PackageManager::PackageManager() {
	// determine data directory
	data_directory = program_dir();
	// check if this is the actual data directory, especially during debugging,
	// the data may be higher up:
	//  exe path  = mse/build/debug/mse.exe
	//  data path = mse/data
	while (!wxDirExists(data_directory + _("/data"))) {
		String d = data_directory;
		data_directory = wxPathOnly(data_directory);
		if (d == data_directory) {
			// we are at the root -> 'data' not found anywhere in the path -> fatal error
			throw Error(_("The MSE data files can not be found, there should be a directory called 'data' with these files"));
		}
	}
	data_directory += _("/data");
}

PackagedP PackageManager::openAny(const String& name) {
	wxFileName fn(data_directory + _("/") + name);
	fn.Normalize();
	String filename = fn.GetFullPath();
	// Is this package already loaded?
	PackagedP& p = loaded_packages[filename];
	if (p) {
		return p;
	} else {
		// load with the right type, based on extension
		if      (fn.GetExt() == _("mse-game"))         p = new_shared<Game>();
//		else if (fn.GetExt() == _("mse-style"))        p = new_shared<CardStyle>();
//		else if (fn.GetExt() == _("mse-locale"))       p = new_shared<Locale>();
//		else if (fn.GetExt() == _("mse-include"))      p = new_shared<IncludePackage>();
//		else if (fn.GetExt() == _("mse-symbol-font"))  p = new_shared<SymbolFont>();
		else {
			throw PackageError(_("Unrecognized package type: ") + fn.GetExt());
		}
		p->open(filename);
		return p;
	}
}

String PackageManager::findFirst(const String& pattern) {
	return wxFindFirstFile(data_directory + _("/") + pattern, 0);
}

void PackageManager::destroy() {
	loaded_packages.clear();
}