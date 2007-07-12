//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/installer.hpp>
#include <util/io/package_manager.hpp>
#include <script/to_value.hpp>
#include <wx/filename.h>

DECLARE_TYPEOF_COLLECTION(String);

// ----------------------------------------------------------------------------- : Installer

String Installer::typeName() const { return _("installer"); }

IMPLEMENT_REFLECTION(Installer) {
	REFLECT_BASE(Packaged);
	REFLECT(packages);
}

// ----------------------------------------------------------------------------- : Installing

void Installer::installFrom(const String& filename, bool message_on_success) {
	Installer i;
	i.open(filename);
	i.install();
	if (message_on_success) {
		wxMessageBox(String::Format(_("'%s' successfully installed %d package%s."), i.name().c_str(), i.packages.size(), i.packages.size() == 1 ? _("") : _("s")),
		             _("Magic Set Editor"), wxOK | wxICON_INFORMATION);
	}
}

void Installer::install() {
	// Walk over all files in this installer
	const FileInfos& file_infos = getFileInfos();
	for (FileInfos::const_iterator it = file_infos.begin() ; it != file_infos.end() ; ++it) {
//		const String& filename = it->first;
		//
	}
	// REMOVE?
	FOR_EACH(p, packages) {
		install(p);
	}
}

void Installer::install(const String& package) {
	// 1. Make sure the package is not loaded
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
