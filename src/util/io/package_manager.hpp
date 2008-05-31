//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_PACKAGE_MANAGER
#define HEADER_UTIL_IO_PACKAGE_MANAGER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <wx/filename.h>

DECLARE_POINTER_TYPE(Packaged);
DECLARE_POINTER_TYPE(PackageVersion);
DECLARE_POINTER_TYPE(InstallablePackage);
class PackageDependency;

// ----------------------------------------------------------------------------- : PackageVersion

/*

/// Information on a package in a repository
class PackageVersionData : public IntrusivePtrVirtualBase {
  public:
	PackageVersionData() {}
	
	String  name;						///< Name of the package
	String  short_name;					///< Name to show on package list.
	String  description;				///< html description
	bool    hidden;						///< Not shown in the package list (installed automatically as a dependency)
	
	PackageVersionP base_version;		///< The locally installed version (if installed)
	PackageVersionP local_version;		///< Modifications made to the locally installed version?
	PackageVersionP remote_version;		///< The version available from the server (if available)
	bool global;						///< The installed package is in the global location, not the user-local one
	bool new_global;					///< 
	
	bool source_local;					///< Is the source
	String source;						///< Where can the package be downloaded/found?
	
	DECLARE_REFLECTION();
};

class UpdateData {
	vector<PackageDependencyP> packages;         ///< package/latest-version-number pairs
	String                     new_updates_url;  ///< updates url has changed? 
};

IMPLEMENT_REFLECTION_NO_SCRIPT(UpdateData) {
	REFLECT_NO_SCRIPT(packages);
	REFLECT_NO_SCRIPT(new_updates_url);
}
*/

// ----------------------------------------------------------------------------- : PackageDirectory

/// A directory for packages
class PackageDirectory {
  public:
	void init(bool local);
	void init(const String& dir);
	
	/// Name of a package in this directory
	String name(const String& name) const;
	/// Does a package with the given name exist?
	bool exists(const String& name) const;
	
	/// Find all packages that match a filename pattern (using wxFindFirst)
	String findFirstMatching(const String& pattern) const;
	
	/// Get all installed packages
	void installedPackages(vector<InstallablePackageP>& packages);
	
	/// Install/uninstall a package
	bool install(const InstallablePackage& package);
	
	void loadDatabase();
	void saveDatabase();
  private:
	bool   is_local;
	String directory;
	vector<PackageVersionP> packages; // sorted by name
	
	String databaseFile();
	// Do the actual installation of a package
	bool actual_install(const InstallablePackage& package, const String& install_dir);
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : PackageManager

/// Package manager, loads data files from the default data directory.
/** The PackageManager ensures that each package is only loaded once.
 *  There is a single global instance of the PackageManager, called packages
 */
class PackageManager {
  public:
	/// Initialize the package manager
	void init();
	/// Empty the list of packages.
	/** This function MUST be called before the program terminates, otherwise
	 *  we could get into fights with pool allocators used by ScriptValues */
	void destroy();
	/// Empty the list of packages, they will all be reloaded
	void reset();
	
	// --------------------------------------------------- : Packages in memory
	
	/// Open a package with the specified name (including extension)
	template <typename T>
	intrusive_ptr<T> open(const String& name) {
		PackagedP p = openAny(name);
		intrusive_ptr<T> typedP = dynamic_pointer_cast<T>(p);
		if (typedP) {
			return typedP;
		} else {
			throw InternalError(format_string(_("Package %s loaded as wrong type"),name));
		}
	}
	
	/// Open a package with the specified name, the type of package is determined by its extension!
	/** @param if just_header is true, then the package is not fully parsed.
	 */
	PackagedP openAny(const String& name, bool just_header = false);
	
	/// Find all packages that match a filename pattern, store them in out
	/** Only reads the package headers */
	void findMatching(const String& pattern, vector<PackagedP>& out);
	
	/// Open a file from a package, with a name encoded as "/package/file"
	/** If 'package' is set then:
	 *    - tries to open a relative file from the package if the name is "file"
	 *    - verifies a dependency from that package if an absolute filename is used
	 *      this is to force people to fill in the dependencies
	 *  Afterwards, package will be set to the package the file is opened from
	 */
	InputStreamP openFileFromPackage(Packaged*& package, const String& name);
	
	// --------------------------------------------------- : Packages on disk
	
	/// Check if the given dependency is currently installed
	bool checkDependency(const PackageDependency& dep, bool report_errors = true);
	/// Determine the latest version of the given package
	/** If it is not installed, returns false */
	bool installedVersion(const String& pkg, Version& version_out);
	
	/// Get all installed packages
	void findAllInstalledPackages(vector<InstallablePackageP>& packages);
	
	/// Install/uninstall a package, returns success
	bool install(const InstallablePackage& package);
	
	// --------------------------------------------------- : Packages on a server
	
  private:
	map<String, PackagedP> loaded_packages;
	PackageDirectory local, global;
};

/// The global PackageManager instance
extern PackageManager package_manager;

// ----------------------------------------------------------------------------- : PackageVersion

/// Version information for an installed package
class PackageVersion : public IntrusivePtrBase<PackageVersion> {
  public:
	PackageVersion() : status(0) {}
	PackageVersion(int status) : status(status) {}
	
	String  name;
	Version version;
	vector<PackageDependencyP> dependencies;
	enum Status
	{	STATUS_LOCAL    = 0x01 ///< Package is installed in the local package dir
	,	STATUS_GLOBAL   = 0x02 ///< Package is installed in the global package dir
	,	STATUS_BLESSED  = 0x10 ///< Package is an official version, installed with the installer
	,	STATUS_MODIFIED = 0x20 ///< Package has been modified after installing
	,	STATUS_FIXED    = 0x40 ///< The package can not be uninstalled
	};
	int status;
	
	/// Check the status of the files in this package
	void check_status(Packaged& package);
	/// Set blessed status to true
	void bless();
	
  public:
	/// Status of a single file
	enum FileStatus
	{	FILE_UNCHANGED
	,	FILE_MODIFIED
	,	FILE_ADDED
	,	FILE_DELETED
	};
	/// Information on files in this package
	struct FileInfo {
		inline FileInfo() {}
		inline FileInfo(const String& file, const DateTime& time, FileStatus status)
			: file(file), time(time), status(status)
		{}
		String     file;
		DateTime   time;
		FileStatus status;
		inline bool operator < (const FileInfo& f) const { return file < f.file; }
	};
  private:
	vector<FileInfo> files; // sorted by filename
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
