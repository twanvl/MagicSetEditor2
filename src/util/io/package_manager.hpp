//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_PACKAGE_MANAGER
#define HEADER_UTIL_IO_PACKAGE_MANAGER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <wx/filename.h>

DECLARE_POINTER_TYPE(Packaged);
class PackageDependency;

// ----------------------------------------------------------------------------- : PackageManager

/// Package manager, loads data files from the default data directory
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
	
	/// Open a package with the specified name (including extension)
	template <typename T>
	intrusive_ptr<T> open(const String& name) {
		wxFileName loc(local_data_directory + _("/") + name);
		loc.Normalize();
		String filename = loc.GetFullPath();
		if (!wxFileExists(filename) && !wxDirExists(filename)) {
			wxFileName glob(global_data_directory + _("/") + name);
			glob.Normalize();
			filename = glob.GetFullPath();
		}
		// Is this package already loaded?
		PackagedP& p = loaded_packages[filename];
		intrusive_ptr<T> typedP = dynamic_pointer_cast<T>(p);
		if (typedP) {
			typedP->loadFully();
			return typedP;
		} else {
			// not loaded, or loaded with wrong type (i.e. with just_header)
			typedP = new_intrusive<T>();
			typedP->open(filename);
			p = typedP;
			return typedP;
		}
	}
	
	/// Open a package with the specified name, the type of package is determined by its extension!
	/** @param if just_header is true, then the package is not fully parsed.
	 */
	PackagedP openAny(const String& name, bool just_header = false);
	
	/// Find all packages that match a filename pattern, store them in out
	/** Only reads the package headers */
	void findMatching(const String& pattern, vector<PackagedP>& out);
	
	/// Open a file from a package, with a name encoded as "package/file"
	InputStreamP openFileFromPackage(const String& name);
	
	/// Check if the given dependency is currently installed
	bool checkDependency(const PackageDependency& dep, bool report_errors = false);

	inline String getGlobalDataDir() const { return global_data_directory; }
	inline String getLocalDataDir() const { return local_data_directory; }
	
  private:
	map<String, PackagedP> loaded_packages;
	String global_data_directory;
	String local_data_directory;
};

/// The global PackageManager instance
extern PackageManager packages;

// ----------------------------------------------------------------------------- : EOF
#endif
