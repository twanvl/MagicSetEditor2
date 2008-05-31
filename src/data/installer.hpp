//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_INSTALLER
#define HEADER_DATA_INSTALLER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>

DECLARE_POINTER_TYPE(Installer);
DECLARE_POINTER_TYPE(PackageVersion);
DECLARE_POINTER_TYPE(PackageDescription);
DECLARE_POINTER_TYPE(DownloadableInstaller);
DECLARE_POINTER_TYPE(InstallablePackage);

// The installer system consists of several layers:
//  - Installer              = an actual package available in memory, containing packages to be installed
//  - DownloadableInstaller  = an installar (possibly) not yet available, i.e. just its URL
//  - PackageDescription     = description of a package version
//  - InstallablePackage     = the complete status of a package, both local and remote


// ----------------------------------------------------------------------------- : Installer

/// A package that contains other packages that can be installed
/** Installers will be sent around the internet, etc. so they are fairly selfcontained.
 */
class Installer : public Packaged {
  public:
	String prefered_filename;	///< What filename should be used (by default), when creating the installer
	vector<PackageDescriptionP> packages;	///< Packages to install
	
	/// Add a package to the installer (if it is not already added).
	/** If the package is named *.mse-installer uses it as the filename instead */
	void addPackage(const String& package);
	/// Add a package to the installer (if it is not already added).
	/** The first package gives the name of the installer.
	 */
	void addPackage(Packaged& package);
	
  protected:
	String typeName() const;
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Installer descriptions

/// A description of a package in an installer.
/** This is essentially the same information as in a Packaged object.
 *  TODO: try to merge these!
 */
class PackageDescription : public IntrusivePtrBase<PackageDescription> {
  public:
	PackageDescription();
	PackageDescription(const Packaged& package);
	
	String name;				///< Filename of the package
	Version version;			///< Version number of this package
	String short_name;			///< Short name of this package
	String full_name;			///< Name of this package, for menus etc.
	String icon_url;			///< Filename or URL of icon to use in package lists
	Image  icon;				///< Icon for the package
	String installer_group;		///< Where to put this package in the installer
	int    position_hint;		///< A hint for the package list
	String description;			///< Changelog/description
	vector<PackageDependencyP> dependencies;	///< Dependencies of this package
	
	/// Merge two descriptions a package. This package takes precedence
	/** Usually one of the descriptions will refer to the locally installed one, the other to the new one */
	void merge(const PackageDescription& p2);
	
	DECLARE_REFLECTION();
};

/// A description of the contents of an installer
class InstallerDescription : public IntrusivePtrBase<InstallerDescription> {
  public:
	vector<PackageDescriptionP> packages;
	
	DECLARE_REFLECTION();
};

/// Information on an installer that can be downloaded
class DownloadableInstaller : public IntrusivePtrBase<DownloadableInstaller> {
  public:
	DownloadableInstaller() : downloadable(true) {}
	DownloadableInstaller(const InstallerP& installer);
	~DownloadableInstaller();
	
	InstallerP installer;      ///< The installer, if it is loaded
	String     installer_url;  ///< The URL where the installer can be found
	String     installer_file; ///< The temp file where the installer can be found (after downloading)
	bool       downloadable;   ///< Is the installer downloadable (in)directly from that url?
	vector<PackageDescriptionP> packages; ///< Packages provided by this installer
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Installable package

/// Installation status of a package
enum PackageStatus
{	PACKAGE_NOT_INSTALLED = 0x0000
,	PACKAGE_INSTALLED     = 0x0001
,	PACKAGE_REMOVABLE     = 0x0002
,	PACKAGE_INSTALLER     = 0x0010 ///< Package can be installed (there is an installer)
,	PACKAGE_INSTALLABLE   = 0x0110 ///< Package can be installed (it makes sense to do so)
,	PACKAGE_UPDATES       = 0x0111 ///< Remote updates available
,	PACKAGE_MODIFIED      = 0x1001 ///< Local changes made
,	PACKAGE_MISSING_DEP   = 0x0200 ///< Missing a dependency for installation
,	PACKAGE_CONFLICTS     = PACKAGE_UPDATES | PACKAGE_MODIFIED
};

/// (un)install a package?
enum PackageAction
{	PACKAGE_NOTHING = 0x001  ///< Don't change anything
,	PACKAGE_INSTALL = 0x002  ///< Install or upgrade the package
,	PACKAGE_REMOVE  = 0x004  ///< Remove the package (if it was installed)
,	PACKAGE_LOCAL   = 0x010  ///< In the local package directory
,	PACKAGE_GLOBAL  = 0x020  ///< In the global package directory
,	PACKAGE_WHERE   = PACKAGE_LOCAL | PACKAGE_GLOBAL
};
// bit twidling
inline PackageAction operator | (PackageAction a, PackageAction b) { return (PackageAction)((int)a | (int) b); }
inline bool flag(int flags, int flag) { return (flags & flag) == flag; }

/// A package that can be installed, or is already installed
class InstallablePackage : public IntrusivePtrVirtualBase {
  public:
	/// A new package
	InstallablePackage(const PackageDescriptionP&, const DownloadableInstallerP&);
	/// An installed package
	InstallablePackage(const PackageDescriptionP&, const PackageVersionP&);
	
	PackageDescriptionP    description; ///< The details of the package. Either from the installed package or from an installer
	PackageVersionP        installed;   ///< The information of the installed package (if installed)
	DownloadableInstallerP installer;   ///< The installer to install from (if updates are available)
	PackageStatus          status;      ///< Status of installation
	PackageAction          action;      ///< What to do with this package?
	
	int automatic; ///< Install/upgrade/remove automaticly to satisfy this many packages
	
	PackageAction old_action;
	int           old_automatic;
	
	void determineStatus();
	
	/// After the action, will the package be installed?
	bool willBeInstalled() const;
	
	/// Is the action possible?
	bool can(PackageAction act) const;
	/// Is the action currently selected?
	bool has(PackageAction act) const;
	
	/// Merge two descriptions of installable packages
	void merge(const InstallablePackage& p2);
};


typedef vector<InstallablePackageP> InstallablePackages;

/// Sort a list of InstallablePackages by package name
void sort(InstallablePackages& packages);

/// Merge two lists of InstallablePackages.
/** The first list contains installed packages, the second list contains packages in an installer
 *  @pre both lists are sorted by package name
 *  @post the output will be sorted
 */
void merge(InstallablePackages& installed, const InstallablePackages& from_installer);

/// Merge the packages from a DownloadableInstaller into a list of InstallablePackages.
void merge(InstallablePackages& installed, const DownloadableInstallerP& installer);

/// Set the action to perform on a given package, makes sure the dependencies are also set correctly
/** Returns true on success
 *  action may be PACKAGE_NOTHING to clear the action
 */
bool set_package_action(InstallablePackages& packages, const InstallablePackageP& package, PackageAction action);

// ----------------------------------------------------------------------------- : Program package

/// The "magicseteditor.exe" package is special, it refers to the program
extern String mse_package;

InstallablePackageP mse_installable_package();

// ----------------------------------------------------------------------------- : EOF
#endif
