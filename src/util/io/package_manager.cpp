//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/package_manager.hpp>
#include <util/error.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <data/locale.hpp>
#include <data/export_template.hpp>
#include <wx/stdpaths.h>

// ----------------------------------------------------------------------------- : PackageManager

PackageManager packages;


void PackageManager::init() {
	// determine data directory
	global_data_directory = wxStandardPaths::Get().GetDataDir();
	// check if this is the actual data directory, especially during debugging,
	// the data may be higher up:
	//  exe path  = mse/build/debug/mse.exe
	//  data path = mse/data
	while (!wxDirExists(global_data_directory + _("/data"))) {
		String d = global_data_directory;
		global_data_directory = wxPathOnly(global_data_directory);
		if (d == global_data_directory) {
			// we are at the root -> 'data' not found anywhere in the path -> fatal error
			throw Error(_("The global MSE data files can not be found, there should be a directory called 'data' with these files. The expected directory to find it in was ") + wxStandardPaths::Get().GetDataDir());
		}
	}
	global_data_directory += _("/data");
	// It's not an error for the local directory not to exist.
	local_data_directory = wxStandardPaths::Get().GetUserDataDir();
	local_data_directory += _("/data");
}

PackagedP PackageManager::openAny(const String& name, bool just_header) {
	// Attempt to load local data first.
	String filename;
	wxFileName fn;
	if (wxFileName(name).IsRelative()) {
		// local data dir?
		fn.Assign(local_data_directory + _("/") + name);
		fn.Normalize();
		filename = fn.GetFullPath();
		if (!wxFileExists(filename) && !wxDirExists(filename)) {
			// global data dir
			fn.Assign(global_data_directory + _("/") + name);
			fn.Normalize();
			filename = fn.GetFullPath();
		}
	} else { // Absolute filename
		fn.Assign(name);
		fn.Normalize();
		filename = fn.GetFullPath();
	}

	// Is this package already loaded?
	PackagedP& p = loaded_packages[filename];
	if (!p) {
		// load with the right type, based on extension
		if      (fn.GetExt() == _("mse-game"))            p = new_intrusive<Game>();
		else if (fn.GetExt() == _("mse-style"))           p = new_intrusive<StyleSheet>();
		else if (fn.GetExt() == _("mse-locale"))          p = new_intrusive<Locale>();
		else if (fn.GetExt() == _("mse-include"))         p = new_intrusive<IncludePackage>();
		else if (fn.GetExt() == _("mse-symbol-font"))     p = new_intrusive<SymbolFont>();
		else if (fn.GetExt() == _("mse-export-template")) p = new_intrusive<ExportTemplate>();
		else {
			throw PackageError(_("Unrecognized package type: '") + fn.GetExt() + _("'\nwhile trying to open: ") + name);
		}
		p->open(filename, just_header);
	}
	return p;
}

void PackageManager::findMatching(const String& pattern, vector<PackagedP>& out) {
	String file;
	// first find local packages
	if (wxDirExists(local_data_directory)) {
		String file = wxFindFirstFile(local_data_directory + _("/") + pattern, 0);
		while (!file.empty()) {
			out.push_back(openAny(file, true));
			file = wxFindNextFile();
		}
	}
	// then global packages not already in the list
	file = wxFindFirstFile(global_data_directory + _("/") + pattern, 0);
	while (!file.empty()) {
		PackagedP p = openAny(file, true);
		if (find(out.begin(), out.end(), p) == out.end()) {
			out.push_back(p);
		}
		file = wxFindNextFile();
	}
}

InputStreamP PackageManager::openFileFromPackage(const String& name) {
	// we don't want an absolute path (for security reasons)
	String n;
	if (!name.empty() && name.GetChar(0) == _('/')) n = name.substr(1);
	else                                            n = name;
	// break
	size_t pos = n.find_first_of(_("/\\"));
	if (pos == String::npos) throw FileNotFoundError(n, _("No package name specified, use 'package/filename'"));
	// open package and file
	PackagedP p = openAny(n.substr(0, pos));
	return p->openIn(n.substr(pos+1));
}

bool PackageManager::checkDependency(const PackageDependency& dep, bool report_errors) {
	// try local package
	String name = local_data_directory + _("/") + dep.package;
	if (!wxFileExists(name) && !wxDirExists(name)) {
		// try global package
		name = global_data_directory + _("/") + dep.package;
		if (!wxFileExists(name) && !wxDirExists(name)) {
			handle_warning(_ERROR_1_("package not found", dep.package),false);
			return false;
		}
	}
	PackagedP package = openAny(dep.package, true);
	if (package->version < dep.version) {
		handle_warning(_ERROR_3_("package out of date", dep.package, package->version.toString(), dep.version.toString()),false);
		return false;
	}
	return true;
}

void PackageManager::destroy() {
	loaded_packages.clear();
}
