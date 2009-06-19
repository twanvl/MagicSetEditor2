//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <util/error.hpp>
#include <util/file_utils.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <data/locale.hpp>
#include <data/export_template.hpp>
#include <data/installer.hpp>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

DECLARE_TYPEOF_COLLECTION(InstallablePackageP);
DECLARE_TYPEOF_COLLECTION(PackageVersionP);
DECLARE_TYPEOF_COLLECTION(PackageVersion::FileInfo);

// ----------------------------------------------------------------------------- : PackageManager : in memory

PackageManager package_manager;


void PackageManager::init() {
	local.init(true);
	global.init(false);
	if (!(local.valid() || global.valid()))
		throw Error(_("The MSE data files can not be found, there should be a directory called 'data' with these files. ")
								_("The expected place to find it in was either ") + wxStandardPaths::Get().GetDataDir() + _(" or ") +
								wxStandardPaths::Get().GetUserDataDir());
}
void PackageManager::destroy() {
	loaded_packages.clear();
}
void PackageManager::reset() {
	loaded_packages.clear();
}

PackagedP PackageManager::openAny(const String& name_, bool just_header) {
	String name = trim(name_);
	if (starts_with(name,_("/"))) name = name.substr(1);
	if (starts_with(name,_(":NO-WARN-DEP:"))) name = name.substr(13);
	// Attempt to load local data first.
	String filename;
	if (wxFileName(name).IsRelative()) {
		// local data dir?
		filename = normalize_filename(local.name(name));
		if (!wxFileExists(filename) && !wxDirExists(filename)) {
			// global data dir
			filename = normalize_filename(global.name(name));
		}
	} else { // Absolute filename
		filename = normalize_filename(name);
	}

	// Is this package already loaded?
	PackagedP& p = loaded_packages[filename];
	if (!p) {
		// load with the right type, based on extension
		wxFileName fn(filename);
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
	} else if (!just_header) {
		p->loadFully();
	}
	return p;
}

void PackageManager::findMatching(const String& pattern, vector<PackagedP>& out) {
	// first find local packages
	String file = local.findFirstMatching(pattern);
	while (!file.empty()) {
		out.push_back(openAny(file, true));
		file = wxFindNextFile();
	}
	// then global packages not already in the list
	file = global.findFirstMatching(pattern);
	while (!file.empty()) {
		PackagedP p = openAny(file, true);
		if (find(out.begin(), out.end(), p) == out.end()) {
			out.push_back(p);
		}
		file = wxFindNextFile();
	}
}

InputStreamP PackageManager::openFileFromPackage(Packaged*& package, const String& name) {
	if (!name.empty() && name.GetChar(0) == _('/')) {
		// absolute name; break name
		size_t start = name.find_first_not_of(_("/\\"), 1); // allow "//package/name" from incorrect scripts
		size_t pos   = name.find_first_of(_("/\\"), start);
		if (start < pos && pos != String::npos) {
			// open package
			PackagedP p = openAny(name.substr(start, pos-start));
			if (package && !is_substr(name,start,_(":NO-WARN-DEP:"))) {
				package->requireDependency(p.get());
			}
			package = p.get();
			return p->openIn(name.substr(pos + 1));
		}
	} else if (package) {
		// relative name
		return package->openIn(name);
	}
	throw FileNotFoundError(name, _("No package name specified, use '/package/filename'"));
}

String PackageManager::openFilenameFromPackage(Packaged*& package, const String& name) {
	if (!name.empty() && name.GetChar(0) == _('/')) {
		// absolute name; break name
		size_t start = name.find_first_not_of(_("/\\"), 1); // allow "//package/name" from incorrect scripts
		size_t pos   = name.find_first_of(_("/\\"), start);
		if (start < pos && pos != String::npos) {
			// open package
			PackagedP p = openAny(name.substr(start, pos-start));
			if (package && !is_substr(name,start,_(":NO-WARN-DEP:"))) {
				package->requireDependency(p.get());
			}
			package = p.get();
			return p->absoluteFilename() + _("/") + name.substr(pos + 1);
		}
	} else if (package) {
		// relative name
		return package->absoluteFilename() + _("/") + name;
	}
	throw FileNotFoundError(name, _("No package name specified, use '/package/filename'"));
}

String PackageManager::getDictionaryDir(bool l) const {
	String dir = (l ? local : global).getDirectory();
	if (dir.empty()) return wxEmptyString;
	else             return dir + _("/dictionaries/");
}

// ----------------------------------------------------------------------------- : PackageManager : on disk

bool PackageManager::checkDependency(const PackageDependency& dep, bool report_errors) {
	// mse package?
	if (dep.package == mse_package) {
		if (app_version < dep.version) {
			handle_warning(_ERROR_3_("package out of date", _("Magic Set Editor"), app_version.toString(), dep.version.toString()),false);
		}
		return true;
	}
	// does the package exist?
	if (!local.exists(dep.package) && !global.exists(dep.package)) {
		if (report_errors)
			handle_warning(_ERROR_1_("package not found", dep.package),false);
		return false;
	}
	PackagedP package = openAny(dep.package, true);
	if (package->version < dep.version) {
		if (report_errors)
			handle_warning(_ERROR_3_("package out of date", dep.package, package->version.toString(), dep.version.toString()),false);
		return false;
	}
	return true;
}
bool PackageManager::installedVersion(const String& package_name, Version& version_out) {
	if (package_name == mse_package) {
		version_out = app_version;
		return true;
	} else {
		if (!local.exists(package_name) && !global.exists(package_name)) return false;
		PackagedP package = openAny(package_name, true);
		version_out = package->version;
		return true;
	}
}

void PackageManager::findAllInstalledPackages(vector<InstallablePackageP>& packages) {
	// from directories
	vector<InstallablePackageP> more_packages;
	global.installedPackages(packages);
	local.installedPackages(more_packages);
	merge(packages, more_packages);
	// the magic appliation package
	packages.push_back(mse_installable_package());
	// invariant: sorted:
	sort(packages);
}

bool PackageManager::install(const InstallablePackage& package) {
	bool install_local = package.has(PACKAGE_ACT_LOCAL);
	return (install_local ? local : global).install(package);
}

// ----------------------------------------------------------------------------- : PackageDirectory

void PackageDirectory::init(bool local) {
	is_local = local;
	if (local) {
		init(wxStandardPaths::Get().GetUserDataDir() + _("/data"));
	} else {
		// determine data directory
		String dir = wxStandardPaths::Get().GetDataDir();
		// check if this is the actual data directory, especially during debugging,
		// the data may be higher up:
		//  exe path  = mse/build/debug/mse.exe
		//  data path = mse/data
		while (!wxDirExists(dir + _("/data"))) {
			String d = dir;
			dir = wxPathOnly(dir);
			if (d == dir) {
				// we are at the root -> 'data' not found anywhere in the path
				dir = wxStandardPaths::Get().GetDataDir();
				break;
			}
		}
		init(dir + _("/data"));
	}
}
void PackageDirectory::init(const String& dir) {
	if (wxDirExists(dir))
		directory = dir;
	else
		directory.clear();
}

String PackageDirectory::name(const String& name) const {
	return directory + _("/") + name;
}
bool PackageDirectory::exists(const String& filename) const {
	String fn = name(filename);
	return wxFileExists(fn) || wxDirExists(fn);
}

String PackageDirectory::findFirstMatching(const String& pattern) const {
	if (!wxDirExists(directory)) return String();
	return wxFindFirstFile(directory + _("/") + pattern, 0);
}

bool compare_name(const PackageVersionP& a, const PackageVersionP& b) {
	return a->name < b->name;
}

void PackageDirectory::installedPackages(vector<InstallablePackageP>& packages_out) {
	loadDatabase();
	// find all package files
	vector<String> in_dir;
	for (String s = findFirstMatching(_("*.mse-*")) ; !s.empty() ; s = wxFindNextFile()) {
		size_t pos = s.find_last_of(_("/\\"));
		if (pos != String::npos) s = s.substr(pos+1);
		// TODO : check for valid package names
		in_dir.push_back(s);
	}
	sort(in_dir.begin(), in_dir.end());
	// merge with package database
	bool db_changed = false;
	vector<PackageVersionP>::const_iterator it1 = packages.begin();
	vector<String>::const_iterator          it2 = in_dir.begin();
	while (it2 != in_dir.end()) {
		if (it1 == packages.end() || (*it1)->name > *it2) {
			// add new package to db
			try {
				PackagedP pack = package_manager.openAny(*it2, true);
				db_changed = true;
				PackageVersionP ver(new PackageVersion(
					is_local ? PackageVersion::STATUS_LOCAL : PackageVersion::STATUS_GLOBAL));
				ver->check_status(*pack);
				packages_out.push_back(new_intrusive2<InstallablePackage>(new_intrusive1<PackageDescription>(*pack), ver));
			} catch (const Error&) {}
			++it2;
		} else if ((*it1)->name < *it2) {
			// delete package from db
			db_changed = true;
			++it1;
		} else {
			// ok, a package already in the db
			try {
				PackagedP pack = package_manager.openAny(*it2, true);
				(*it1)->check_status(*pack);
				packages_out.push_back(new_intrusive2<InstallablePackage>(new_intrusive1<PackageDescription>(*pack), *it1));
			} catch (const Error&) { db_changed = true; }
			++it1, ++it2;
		}
	}
	if (it1 != packages.end()) db_changed = true;
	// has the database of installed packages changed?
	if (db_changed) {
		packages.clear();
		FOR_EACH(p, packages_out) {
			if (p->installed) packages.push_back(p->installed);
		}
		saveDatabase();
	}
}

void PackageDirectory::bless(const String& package_name) {
	PackagedP pack = package_manager.openAny(package_name, true);
	// already have this package?
	FOR_EACH(ver, packages) {
		if (ver->name == package_name) {
			ver->check_status(*pack);
			ver->bless();
			return;
		}
	}
	// a new package
	PackageVersionP ver(new PackageVersion(
		is_local ? PackageVersion::STATUS_LOCAL : PackageVersion::STATUS_GLOBAL));
	ver->check_status(*pack);
	ver->bless();
	packages.push_back(ver);
	sort(packages.begin(), packages.end(), compare_name);
}

void PackageDirectory::removeFromDatabase(const String& package_name) {
	size_t i = 0, j = 0;
	for ( ; i < packages.size() ; ++i) {
		if (packages[i]->name != package_name) {
			packages[j++] = packages[i];
		}
	}
	packages.resize(j);
}

IMPLEMENT_REFLECTION(PackageDirectory) {
	REFLECT(packages);
}

void PackageDirectory::loadDatabase() {
	if (!packages.empty()) return;
	String filename = databaseFile();
	if (wxFileExists(filename)) {
		// packages file not existing is not an error
		shared_ptr<wxFileInputStream> file = new_shared1<wxFileInputStream>(filename);
		if (!file->Ok()) return; // failure is not an error
		Reader reader(file, nullptr, filename);
		reader.handle_greedy(*this);
		sort(packages.begin(), packages.end(), compare_name);
	}
}

void PackageDirectory::saveDatabase() {
	Writer writer(new_shared1<wxFileOutputStream>(databaseFile()), app_version);
	writer.handle(*this);
}
String PackageDirectory::databaseFile() {
	return name(_("packages"));
}

// ----------------------------------------------------------------------------- : PackageDirectory : installing

bool PackageDirectory::install(const InstallablePackage& package) {
	String n = name(package.description->name);
	if (package.action & PACKAGE_ACT_REMOVE) {
		if (!remove_file_or_dir(n)) return false;
		removeFromDatabase(package.description->name);
	} else if (package.action & PACKAGE_ACT_INSTALL) {
		if (!remove_file_or_dir(n + _(".new"))) return false;
		bool ok = actual_install(package, n + _(".new"));
		if (!ok) return false;
		move_ignored_files(n, n + _(".new")); // copy over files from the old installed version to the new one
		if (!remove_file_or_dir(n)) return false;
		if (!rename_file_or_dir(n + _(".new"), n)) return false;
		bless(package.description->name);
	}
	saveDatabase();
	return true;
}

bool PackageDirectory::actual_install(const InstallablePackage& package, const String& install_dir) {
	String name = package.description->name;
	if (!package.installer->installer) {
		handle_warning(_("Installer not found for package: ") + name);
		return false;
	}
	Installer& installer = *package.installer->installer;
	// install files
	const Packaged::FileInfos& file_infos = installer.getFileInfos();
	for (Packaged::FileInfos::const_iterator it = file_infos.begin() ; it != file_infos.end() ; ++it) {
		String file = it->first;
		if (!is_substr_i(file,0,name)) continue; // not the right package
		// correct filename
		String local_file = install_dir + file.substr(name.length());
		create_parent_dirs(local_file);
		// copy file
		InputStreamP is = installer.openIn(file);
		wxFileOutputStream os (local_file);
		if (!os.IsOk()) {
			int act = wxMessageBox(_ERROR_1_("cannot create file", file), _TITLE_("cannot create file"), wxICON_ERROR | wxYES_NO);
			if (act == wxNO) return false;
		}
		os.Write(*is);
	}
	// update package database
	// TODO: bless the package?
	return true;
}

// ----------------------------------------------------------------------------- : PackageVersion

template <> void Writer::handle(const PackageVersion::FileInfo& f) {
	if (f.status == PackageVersion::FILE_DELETED) {
		handle(_("D ") + f.file);
	} else {
		handle(format_string(_("%s%s %s"),
			  f.status == PackageVersion::FILE_ADDED    ? _("A")
			: f.status == PackageVersion::FILE_MODIFIED ? _("M") : _(""),
			f.time.Format(_("%Y%m%dT%H%M%S")),
			f.file));
	}
}
template <> void Reader::handle(PackageVersion::FileInfo& f) {
	String s; handle(s);
	// read status
	if (s.size() < 2) {f.status = PackageVersion::FILE_ADDED; return; }
	f.status = s.GetChar(0) == _('M') ? PackageVersion::FILE_MODIFIED
	         : s.GetChar(0) == _('A') ? PackageVersion::FILE_ADDED
	         : s.GetChar(0) == _('D') ? PackageVersion::FILE_DELETED
	         :                          PackageVersion::FILE_UNCHANGED;
	if (f.status == PackageVersion::FILE_DELETED) {
		if (s.GetChar(1) != _(' ')) {f.status = PackageVersion::FILE_ADDED; return; }
		f.file = s.substr(2);
		return;
	} else if (f.status != PackageVersion::FILE_UNCHANGED) {
		s = s.substr(1);
	}
	if (s.size() < 8+1+6+1)         {f.status = PackageVersion::FILE_ADDED; return; }
	if (s.GetChar(8+1+6) != _(' ')) {f.status = PackageVersion::FILE_ADDED; return; } // invalid format
	// read time, filename
	f.time.ParseFormat(s, _("%Y%m%dT%H%M%S"));
	f.file = s.substr(8+1+6+1);
}
IMPLEMENT_REFLECTION_NO_SCRIPT(PackageVersion) {
	REFLECT_NO_SCRIPT(name);
	REFLECT_NO_SCRIPT(version);
	REFLECT_NO_SCRIPT(status);
	REFLECT_NO_SCRIPT(files);
}

void PackageVersion::check_status(Packaged& package) {
	status &= ~STATUS_MODIFIED;
	if (!(status & STATUS_BLESSED)) status |= STATUS_MODIFIED;
	name    = package.relativeFilename();
	version = package.version;
	// Merge our files list with the list from the package
	vector<FileInfo> new_files;
	Package::FileInfos fis = package.getFileInfos();
	Package::FileInfos::const_iterator it1 = fis.begin();
	vector<FileInfo>::iterator it2 = files.begin();
//%	size_t it2 = 0, size = files.size();
//%	bool need_sort = false;
	while(it1 != fis.end() || it2 != files.end()) {
		if (it1 != fis.end() && it2 != files.end() && it1->first == it2->file) {
			DateTime mtime = package.modificationTime(*it1);
			if (mtime != it2->time) {
				it2->time   = mtime;
				it2->status = FILE_MODIFIED;
				new_files.push_back(*it2);
				status |= STATUS_MODIFIED;
			}
			++it1; ++it2;
		} else if (it1 != fis.end() && (it2 == files.end() || it1->first < it2->file)) {
			// this is a new file
			DateTime mtime = package.modificationTime(*it1);
			new_files.push_back(FileInfo(it1->first, mtime, FILE_ADDED));
			status |= STATUS_MODIFIED;
			++it1;
		} else {
			// this file is no longer in the package, it was deleted
			if (it2->status != FILE_ADDED) {
				it2->status = FILE_DELETED;
				new_files.push_back(*it2);
				status |= STATUS_MODIFIED;
			}
			++it2;
		}
	}
	swap(files,new_files);
}

inline bool is_deleted(PackageVersion::FileInfo f) {
	return f.status == PackageVersion::FILE_DELETED;
}
void PackageVersion::bless() {
	files.erase(remove_if(files.begin(),files.end(),is_deleted),files.end());
	FOR_EACH(f,files) {
		f.status = FILE_UNCHANGED;
	}
	status &= ~STATUS_MODIFIED;
	status |= STATUS_BLESSED;
}
