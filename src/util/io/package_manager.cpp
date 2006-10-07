//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/package_manager.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : PackageManager

String programDir() {
	return _("."); //TODO
}

PackageManager packages;


PackageManager::PackageManager() {
	// determine data directory
	dataDirectory = programDir();
	// check if this is the actual data directory, especially during debugging,
	// the data may be higher up:
	//  exe path  = mse/build/debug/mse.exe
	//  data path = mse/data
	while (!wxDirExists(dataDirectory + _("/data"))) {
		String d = dataDirectory;
		dataDirectory = wxPathOnly(dataDirectory);
		if (d == dataDirectory) {
			// we are at the root -> 'data' not found anywhere in the path -> fatal error
			throw Error(_("The MSE data files can not be found, there should be a directory called 'data' with these files"));
		}
	}
	dataDirectory += _("/data");
}
