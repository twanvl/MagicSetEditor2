//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/package_manager.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : PackageManager

String program_dir() {
	return _("."); //TODO
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

void PackageManager::destroy() {
	loaded_packages.clear();
}