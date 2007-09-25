//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/installer.hpp>
#include <data/locale.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <data/export_template.hpp>
#include <util/io/package_manager.hpp>
#include <util/platform.hpp>
#include <script/to_value.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/stdpaths.h>

DECLARE_TYPEOF_COLLECTION(String);
DECLARE_TYPEOF_COLLECTION(PackagedP);
DECLARE_TYPEOF_COLLECTION(PackageDependencyP);
DECLARE_POINTER_TYPE(wxFileInputStream);
DECLARE_POINTER_TYPE(wxZipInputStream);

// ----------------------------------------------------------------------------- : Installer

String Installer::typeName() const { return _("installer"); }

IMPLEMENT_REFLECTION(Installer) {
	REFLECT_BASE(Packaged);
	REFLECT(packages);
}

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
	String install_dir = local ? ::packages.getLocalDataDir() : ::packages.getGlobalDataDir();
	if (!wxDirExists(install_dir)) {
		wxMkdir(install_dir, 0755);
	}
	
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
}

void Installer::install(const String& package) {
	// TODO
}

// ----------------------------------------------------------------------------- : Creating

void Installer::addPackage(const String& package) {
	wxFileName fn(package);
	if (fn.GetExt() == _("mse-installer")) {
		prefered_filename = package;
	} else {
		PackagedP p = ::packages.openAny(package);
		addPackage(*p);
	}
}

void Installer::addPackage(Packaged& package) {
	// Add to list of packages
	String name = package.relativeFilename();
	if (find(packages.begin(), packages.end(), package.name()) != packages.end()) {
		return; // already added
	}
	if (prefered_filename.empty()) {
		prefered_filename = package.name() + _(".mse-installer");
	}
	packages.push_back(name);
	// Copy all files from that package to this one
	const FileInfos& file_infos = package.getFileInfos();
	for (FileInfos::const_iterator it = file_infos.begin() ; it != file_infos.end() ; ++it) {
		String file = it->first;
		InputStreamP  is = package.openIn(file);
		OutputStreamP os = openOut(name + _("/") + file);
		os->Write(*is);
	}
}
