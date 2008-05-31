//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/installer.hpp>
#include <data/locale.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <data/export_template.hpp>
#include <util/io/package_manager.hpp>
#include <util/platform.hpp>
#include <gui/util.hpp> // load_resource_image
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/stdpaths.h>

DECLARE_TYPEOF_COLLECTION(String);
DECLARE_TYPEOF_COLLECTION(PackagedP);
DECLARE_TYPEOF_COLLECTION(PackageDependencyP);
DECLARE_TYPEOF_COLLECTION(PackageDescriptionP);
DECLARE_TYPEOF_COLLECTION(InstallablePackageP);
DECLARE_POINTER_TYPE(wxFileInputStream);
DECLARE_POINTER_TYPE(wxZipInputStream);

// Don't do this check for now, because we can't bless packages
#define USE_MODIFIED_CHECK 0

// ----------------------------------------------------------------------------- : Installer

String Installer::typeName() const { return _("installer"); }

IMPLEMENT_REFLECTION(Installer) {
	REFLECT_BASE(Packaged);
	REFLECT(packages);
}

#if 0
// ----------------------------------------------------------------------------- : Installing

void Installer::installFrom(const String& filename, bool message_on_success, bool local) {
	Installer i;
	i.open(filename);
	try {
		i.install(local);
	} catch (const Error& e) {
		handle_error(e);
		return;
	}
	if (message_on_success) {
		//wxMessageBox(_ERROR_2_("successful install", i.name(), String() << i.packaged.size(), 
		wxMessageBox(String::Format(_("'%s' successfully installed %d package%s."), i.name().c_str(), i.packages.size(), i.packages.size() == 1 ? _("") : _("s")),
		             _("Magic Set Editor"), wxOK | wxICON_INFORMATION);
	}
}

struct dependency_check : public unary_function<bool, PackagedP> {
	dependency_check(PackageDependencyP dep) : dep (dep) {}
	bool operator () (PackagedP package) {
		return package->name() == dep->package && package->version >= dep->version;
	}
  private:
	PackageDependencyP dep;
};

void Installer::install(bool local, bool check_dependencies) {
	// Destination directory
//	String install_dir = local ? ::packages.getLocalDataDir() : ::packages.getGlobalDataDir();
	String install_dir = _("TODO");
	if (!wxDirExists(install_dir)) {
		wxMkdir(install_dir, 0755);
	}
	/*
	// All the packages we're installing.
	vector<PackagedP> new_packages;

	FOR_EACH(p, packages) {
		if (wxDirExists(install_dir + _("/") + p) || wxFileExists(install_dir + _("/") + p)) {
			throw PackageError(_("Package ") + p + _(" is already installed. Overwriting currently not supported."));
		}
		PackagedP pack;
		wxString fn(wxFileName(p).GetExt());
		if      (fn == _("mse-game"))            pack = new_intrusive<Game>();
		else if (fn == _("mse-style"))           pack = new_intrusive<StyleSheet>();
		else if (fn == _("mse-locale"))          pack = new_intrusive<Locale>();
		else if (fn == _("mse-include"))         pack = new_intrusive<IncludePackage>();
		else if (fn == _("mse-symbol-font"))     pack = new_intrusive<SymbolFont>();
		else if (fn == _("mse-export-template")) pack = new_intrusive<ExportTemplate>();
		else {
			throw PackageError(_("Unrecognized package type: '") + fn + _("'\nwhile trying to install: ") + p);
		}
		Reader reader(openIn(p + _("/") + pack->typeName()));
		pack->Packaged::reflect_impl(reader);
		new_packages.push_back(pack);
	}
	
	if (check_dependencies) {
		// Check dependencies for each and every package.
		FOR_EACH(p, new_packages) {
			FOR_EACH(d, p->dependencies) {
				if (find_if(new_packages.begin(), new_packages.end(), dependency_check(d)) == new_packages.end() &&
					!::packages.checkDependency(*d, false)) {
					throw PackageError(_("Unmet dependency for package ") + p->relativeFilename() + _(": ") + d->package + _(", version ") + d->version.toString() + _(" or higher."));
				}
			}
		}
	}
	
	const FileInfos& file_infos = getFileInfos();
	for (FileInfos::const_iterator it = file_infos.begin() ; it != file_infos.end() ; ++it) {
		String file = it->first;
		
		wxFileName fn(file);
		wxArrayString dirs = fn.GetDirs();
		
		if (fn.IsDir() || !dirs.GetCount() || find(packages.begin(), packages.end(), dirs[0]) == packages.end()) {
			continue;
		}
		
		String current_dir = install_dir;
		for (size_t j = 0; j < dirs.GetCount(); ++j) {
			current_dir += _("/") + dirs[j];
			if (!wxDirExists(current_dir) && !wxMkdir(current_dir, 0755)) {
				throw PackageError(_("Cannot create folder ") + current_dir + _(" for install. Warning: some packages may have been installed anyway, and some may only be partially installed."));
			}
		}
		
		InputStreamP is = openIn(file);
		wxFileOutputStream os (install_dir + _("/") + file);
		if (!os.IsOk()) {
			throw PackageError(_("Cannot create file ") + install_dir + _("/") + file + _(" for install. Warning: some packages may have been installed anyway, and some may only be partially installed."));
		}
		os.Write(*is);
	}
	*/
}

void Installer::install(const String& package) {
	// TODO
}

#endif

// ----------------------------------------------------------------------------- : Creating

void Installer::addPackage(const String& package) {
	wxFileName fn(package);
	if (fn.GetExt() == _("mse-installer")) {
		prefered_filename = package;
	} else {
		PackagedP p = package_manager.openAny(package);
		addPackage(*p);
	}
}

void Installer::addPackage(Packaged& package) {
	// Add to list of packages
	String name = package.relativeFilename();
	FOR_EACH(p, packages) {
		if (p->name == name) {
			return; // already added
		}
	}
	packages.push_back(new_intrusive1<PackageDescription>(package));
	// use this as a filename?
	if (prefered_filename.empty()) {
		prefered_filename = package.name() + _(".mse-installer");
	}
	// Copy all files from that package to this one
	const FileInfos& file_infos = package.getFileInfos();
	for (FileInfos::const_iterator it = file_infos.begin() ; it != file_infos.end() ; ++it) {
		String file = it->first;
		InputStreamP  is = package.openIn(file);
		OutputStreamP os = openOut(name + _("/") + file);
		os->Write(*is);
	}
}

// ----------------------------------------------------------------------------- : Installer descriptions

PackageDescription::PackageDescription() : position_hint(0) {}
PackageDescription::PackageDescription(const Packaged& package)
	: name(package.relativeFilename())
	, version(package.version)
	, short_name(package.short_name)
	, full_name(package.full_name)
	, icon_url(_(""))
	, installer_group(package.installer_group)
	, position_hint(package.position_hint)
	//, description(package.description)
	, dependencies(package.dependencies)
{
	// name
	if (full_name.empty()) full_name = short_name;
	// installer group
	if (installer_group.empty()) {
		// "game-style.mse-something" -> "game/style_short_name"
		installer_group = package.name();
		size_t pos = installer_group.find_last_of(_('-'));
		if (pos != String::npos) installer_group.resize(pos);
		if (!installer_group.empty()) {
			installer_group = _("unclassified/") + replace_all(installer_group,_("-"),_("/")) + _("/");
		} else {
			installer_group = _("unclassified/");
		}
		if (dynamic_cast<const Game*>(&package)) {
			installer_group += _("Game files");
		} else {
			installer_group += short_name;
		}
	}
	// icon
	InputStreamP file = const_cast<Packaged&>(package).openIconFile();
	if (file) icon.LoadFile(*file);
}

IMPLEMENT_REFLECTION_NO_SCRIPT(PackageDescription) {
	REFLECT(name);
	REFLECT(version);
	REFLECT(short_name);
	REFLECT(full_name);
	REFLECT(icon_url);
	REFLECT(installer_group);
	REFLECT(position_hint);
	REFLECT(description);
	REFLECT_N("depends ons", dependencies);
}

void PackageDescription::merge(const PackageDescription& p2) {
	if (!icon.Ok() && !icon_url) icon = p2.icon;
	if (installer_group.empty()) installer_group = p2.installer_group;
	if (short_name.empty()) short_name = p2.short_name;
	if (full_name.empty()) full_name = p2.full_name;
}

IMPLEMENT_REFLECTION_NO_SCRIPT(InstallerDescription) {
	REFLECT(packages);
}

IMPLEMENT_REFLECTION_NO_SCRIPT(DownloadableInstaller) {
	REFLECT_N("url", installer_url);
	REFLECT(downloadable);
	REFLECT(packages);
}

DownloadableInstaller::DownloadableInstaller(const InstallerP& installer)
	: installer(installer)
	, downloadable(false)
	, packages(installer->packages)
{}

DownloadableInstaller::~DownloadableInstaller() {
	if (!installer_file.empty()) {
		wxRemoveFile(installer_file);
	}
}

// ----------------------------------------------------------------------------- : Installable package

InstallablePackage::InstallablePackage(const PackageDescriptionP& description, const PackageVersionP& installed)
	: description(description)
	, installed(installed)
	, status(PACKAGE_INSTALLED)
	, action(PACKAGE_NOTHING)
{}
InstallablePackage::InstallablePackage(const PackageDescriptionP& description , const DownloadableInstallerP& installer)
	: description(description)
	, installer(installer)
	, status(PACKAGE_INSTALLABLE)
	, action(PACKAGE_NOTHING)
{}

void InstallablePackage::determineStatus() {
	status = PACKAGE_NOT_INSTALLED;
	if (installer) {
		status = (PackageStatus)(status | PACKAGE_INSTALLER);
		if (!installed || installed->version < description->version) {
			status = (PackageStatus)(status | PACKAGE_INSTALLABLE);
		}
	}
	if (installed) {
		status = (PackageStatus)(status | PACKAGE_INSTALLED);
		if (!(installed->status & PackageVersion::STATUS_FIXED)) {
			status = (PackageStatus)(status | PACKAGE_REMOVABLE);
		}
	}
	#if USE_MODIFIED_CHECK
	if (installed && (installed->status & PackageVersion::STATUS_MODIFIED)) {
		status = (PackageStatus)(status | PACKAGE_MODIFIED);
	}
	#endif
}

bool InstallablePackage::willBeInstalled() const {
	return (action & PACKAGE_INSTALL) ||
	       ((status & PACKAGE_INSTALLED) && !(action & PACKAGE_REMOVE));
}
bool InstallablePackage::can(PackageAction act) const {
	if (act & PACKAGE_INSTALL) return flag(status, PACKAGE_INSTALLABLE);
	if (act & PACKAGE_REMOVE) {
		bool ok = flag(status, PACKAGE_REMOVABLE);
		if (!(act & PACKAGE_GLOBAL) && installed && PackageVersion::STATUS_GLOBAL) {
			// package installed globally can't be removed locally
			return false;
		}
		return ok;
	}
	if (act & PACKAGE_NOTHING) {
		return true;
	}
	else return false;
}
bool InstallablePackage::has(PackageAction act) const {
	return (action & act) == act;
}

void InstallablePackage::merge(const InstallablePackage& p) {
	if (!installed) installed = p.installed;
	if (!installer) {
		p.description->merge(*description);
		description = p.description; // installer has new description
		installer = p.installer;
	}
}


bool before(const InstallablePackageP& a, const InstallablePackageP& b) {
	assert(a->description && b->description);
	return a->description->name < b->description->name;
}
void sort(InstallablePackages& packages) {
	sort(packages.begin(), packages.end(), before);
}

void merge(InstallablePackages& list1, const InstallablePackages& list2) {
	InstallablePackages::iterator       it1 = list1.begin();
	InstallablePackages::const_iterator it2 = list2.begin();
	InstallablePackages add;
	while (it1 != list1.end() || it2 != list2.end()) {
		if (it1 != list1.end() && (it2 == list2.end() || before(*it1,*it2))) {
			++it1;
		} else if (it1 == list1.end() || before(*it2,*it1)) {
			add.push_back(*it2);
			++it2;
		} else {
			(*it1)->merge(**it2);
			++it1,++it2;
		}
	}
	if (!add.empty()) {
		list1.insert(list1.end(), add.begin(), add.end());
		sort(list1);
	}
}

void merge(InstallablePackages& installed, const DownloadableInstallerP& installer) {
	InstallablePackages ips;
	FOR_EACH(p, installer->packages) {
		ips.push_back(new_intrusive2<InstallablePackage>(p,installer));
	}
	sort(ips);
	merge(installed, ips);
}


// ----------------------------------------------------------------------------- : Installable package : dependency stuff

InstallablePackage* find_package(InstallablePackages& packages, const String& name) {
	// TODO: The packages are sorted, so we could use a binary search.
	FOR_EACH(p,packages) {
		if (p->description->name == name) return p.get();
	}
	return nullptr;
}

/// A dependency on the new or installed version of an installable package
/// Used only inside these algorithms
struct Dep {
	InstallablePackage* package;
	bool new_version;
};

typedef void (*DependencyFun)(InstallablePackages& packages, Dep dep, PackageAction where);

/// All dependencies of a package(version)
void for_each_dependency(DependencyFun fun, InstallablePackages& packages, Dep pdep, PackageAction where) {
	vector<PackageDependencyP>& deps
		= pdep.new_version
			? pdep.package->installed->dependencies
			: pdep.package->description->dependencies;
	FOR_EACH(dep_,deps) {
		// NOTE: an old version will never depend on a new version
		Dep d = {find_package(packages, dep_->package), pdep.new_version};
		if (d.package && d.package->installed && d.package->installed->version >= dep_->version) {
			d.new_version = false;
		}
		fun(packages,d,where);
	}
}
/// All packages that depend on a package(version)
void for_each_backwards_dependency(DependencyFun fun, InstallablePackages& packages, Dep pdep, PackageAction where) {
	FOR_EACH(p,packages) {
		if (p->installed)
		FOR_EACH(dep_,p->installed->dependencies) {
			if (pdep.package->description->name != dep_->package) {
				continue; // no dependency on p
			}
			if (pdep.new_version && pdep.package->installed && pdep.package->installed->version >= dep_->version) {
				continue; // no need for new version of pdep
			}
			Dep d = {p.get(), false};
			fun(packages,d,where);
		}
		if (p->installer)
		FOR_EACH(dep_,p->description->dependencies) {
			if (pdep.package->description->name != dep_->package) {
				continue; // no dependency on p
			}
			if (pdep.new_version && pdep.package->installed && pdep.package->installed->version >= dep_->version) {
				continue; // no need for new version of pdep
			}
			Dep d = {p.get(), true};
			fun(packages,d,where);
		}
	}
}

void inc_if_nonzero(int& x) { if (x) x++; }

void add_package_dependency(InstallablePackages& packages, Dep dep, PackageAction where) {
	if (!dep.package) return;
	bool change = false;
	if (dep.new_version) {
		change = !(dep.package->action & PACKAGE_INSTALL);
		dep.package->action = where | PACKAGE_INSTALL;
		inc_if_nonzero(dep.package->automatic);
	} else if (dep.package->action & PACKAGE_REMOVE) {
		dep.package->action = where | PACKAGE_NOTHING;
		dep.package->automatic = 0;
	}
	if (change) {
		for_each_dependency(add_package_dependency, packages, dep, where);
	}
}
void remove_package_dependency_need_not_install(InstallablePackages& packages, Dep dep, PackageAction where) {
	if (!dep.package) return;
	if (dep.new_version && (dep.package->action & PACKAGE_INSTALL) && dep.package->automatic) {
		// we no longer need to install this package
		if (--dep.package->automatic == 0) {
			dep.package->action = where | PACKAGE_NOTHING;
			for_each_dependency(remove_package_dependency_need_not_install, packages, dep, where);
		}
	}
	// TODO: also uninstall packages that are already installed, and which are 'hidden'?
}
void remove_package_dependency(InstallablePackages& packages, Dep dep, PackageAction where) {
	if (!dep.package) return;
	bool change = false;
	if (dep.new_version) {
		change = !(dep.package->action & PACKAGE_NOTHING);
		dep.package->action = where | PACKAGE_NOTHING;
		dep.package->automatic = 0;
	} else if (dep.package->status & PACKAGE_REMOVABLE) {
		change = !(dep.package->action & PACKAGE_REMOVE);
		dep.package->action = where | PACKAGE_REMOVE;
		inc_if_nonzero(dep.package->automatic);
	}
	if (change) {
		// things that depend on us
		for_each_backwards_dependency(remove_package_dependency, packages, dep, where);
		// things we depend on
		for_each_dependency(remove_package_dependency_need_not_install, packages, dep, where);
	}
}

// ----------------------------------------------------------------------------- : Installable package : dependency stuff (OLD)

bool add_package_dependency(InstallablePackages& packages, const PackageDependency& dep, PackageAction where, bool set) {
	FOR_EACH(p, packages) {
		if (p->description->name == dep.package) {
			// Some package depends on this package, so install it if needed
			// Mark the installation as "automatically needed for X packages"
			// if !set then instead the dependency is no longer needed because we are not installing the package
			if (!p->installed || p->installed->version < dep.version) {
				bool change = false;
				if (p->action & PACKAGE_INSTALL) {
					// this package is already scheduled for installation
					if (p->automatic) {
						// we are already automatically depending on this package
						p->automatic += set ? +1 : -1;
						if (p->automatic == 0) {
							// no one needs this package anymore
							p->action = PACKAGE_NOTHING;
							change = true;
						}
					}
				} else if (set) {
					p->action = where | PACKAGE_INSTALL;
					p->automatic = 1;
					change = true;
				}
				// recursively add/remove dependencies
				FOR_EACH(dep, p->description->dependencies) {
					if (!add_package_dependency(packages, *dep, where, set)) {
						return false; // failed
					}
				}
			}
			return true;
		}
	}
	return false;
}

void remove_package_dependency(InstallablePackages& packages, const PackageDescription& ver, PackageAction where, bool set) {
	FOR_EACH(p, packages) {
		FOR_EACH(dep, p->description->dependencies) {
			if (dep->package == ver.name) {
				// we can no longer use package p
				if (p->action & PACKAGE_REMOVE) {
					if (p->automatic) {
						p->automatic += set ? +1 : -1;
						if (p->automatic == 0) {
							// no one needs this package anymore
							p->action = PACKAGE_NOTHING;
							remove_package_dependency(packages, *p->description, where, set);
						}
					}
				} else if (set) {
					p->action = where | PACKAGE_REMOVE;
					p->automatic = 1;
					remove_package_dependency(packages, *p->description, where, set);
				}
				break;
			}
		}
	}
}

bool set_package_action_unsafe(InstallablePackages& packages, const InstallablePackageP& package, PackageAction action) {
	PackageAction where = (PackageAction)(action & PACKAGE_WHERE);
	if ((action & PACKAGE_INSTALL) || ((action & PACKAGE_NOTHING) && (package->status & PACKAGE_INSTALLED))) {
		// need the package
		package->automatic = 0;
		package->action    = action;
		// check dependencies
		FOR_EACH(dep, package->description->dependencies) {
			if (!add_package_dependency(packages, *dep, where, !(action & PACKAGE_NOTHING))) return false;
		}
		return true;
	} else if ((action & PACKAGE_REMOVE) || ((action & PACKAGE_NOTHING) && !(package->status & PACKAGE_INSTALLED))) {
		package->automatic = 0;
		package->action    = action;
		// check dependencies
		remove_package_dependency(packages, *package->description, where, !(action & PACKAGE_NOTHING));
		return true;
	}
	return false;
}

bool set_package_action(InstallablePackages& packages, const InstallablePackageP& package, PackageAction action) {
	if (package->has(action)) return false;
	// backup
	FOR_EACH(p,packages) {
		p->old_action    = p->action;
		p->old_automatic = p->automatic;
	}
	// set
	if (set_package_action_unsafe(packages, package, action)) return true;
	// undo
	FOR_EACH(p,packages) {
		p->action    = p->old_action;
		p->automatic = p->old_automatic;
	}
	return false;
}


// ----------------------------------------------------------------------------- : MSE package

String mse_package = _("magicseteditor.exe");

InstallablePackageP mse_installable_package() {
	PackageVersionP mse_version(new PackageVersion(
		PackageVersion::STATUS_GLOBAL |
		PackageVersion::STATUS_BLESSED |
		PackageVersion::STATUS_FIXED));
	mse_version->name    = mse_package;
	mse_version->version = app_version;
	PackageDescriptionP mse_description(new PackageDescription);
	mse_description->name          = mse_package;
	mse_description->short_name    = mse_description->full_name = mse_description->installer_group
	        = _TITLE_("magic set editor");
	mse_description->position_hint = -100;
	mse_description->icon          = load_resource_image(_("installer_program"));
	//mse_description->description   = _LABEL_("magic set editor package");
	return new_intrusive2<InstallablePackage>(mse_description, mse_version);
}
