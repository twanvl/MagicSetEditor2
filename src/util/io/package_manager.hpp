//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_PACKAGE_MANAGER
#define HEADER_UTIL_IO_PACKAGE_MANAGER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <wx/filename.h>

DECLARE_POINTER_TYPE(Packaged);

// ----------------------------------------------------------------------------- : PackageManager

/// Package manager, loads data files from the default data directory
/** The PackageManager ensures that each package is only loaded once.
 *  There is a single global instance of the PackageManager, called packages
 */
class PackageManager {
  public:
	PackageManager();
	
	/// Open a package with the specified name (including extension)
	template <typename T>
	shared_ptr<T> open(const String& name) {
		wxFileName fn(data_directory + _("/") + name);
		fn.Normalize();
		String filename = fn.GetFullPath();
		// Is this package already loaded?
		PackagedP& p = loaded_packages[filename];
		shared_ptr<T> typedP = dynamic_pointer_cast<T>(p);
		if (typedP) {
			return typedP;
		} else {
			// not loaded, or loaded with wrong type
			p = typedP = new_shared<T>();
			typedP->open(filename);
			return typedP;
		}
	}
	
	/// Open a package with the specified name, the type of package is determined by its extension!
	PackagedP openAny(const String& name);
	
	/// Find a package whos name matches a pattern
	/** Find more using wxFindNextFile().
	 *  If no package is found returns an empty string.
	 */
	String findFirst(const String& pattern);
	
	// Open a file from a package, with a name encoded as "package/file"
	InputStreamP openFileFromPackage(const String& name);
	
	/// Empty the list of packages.
	/** This function MUST be called before the program terminates, otherwise
	 *  we could get into fights with pool allocators used by ScriptValues */
	void destroy();
	
  private:
	map<String, PackagedP> loaded_packages;
	String data_directory;
};

/// The global PackageManager instance
extern PackageManager packages;

// ----------------------------------------------------------------------------- : EOF
#endif
