//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_PACKAGE
#define HEADER_UTIL_IO_PACKAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/reflect.hpp>
#include <util/dynamic_arg.hpp>
#include <util/error.hpp>

class Package;
class wxFileInputStream;
class wxZipInputStream;
class wxZipEntry;
DECLARE_POINTER_TYPE(PackageDependency);

/// The package that is currently being written to
DECLARE_DYNAMIC_ARG(Package*, writing_package);
/// The package that is being put onto/read from the clipboard
DECLARE_DYNAMIC_ARG(Package*, clipboard_package);

// ----------------------------------------------------------------------------- : Package

/// A package is a container for files. On disk it is either a directory or a zip file.
/** Specific types of packages should inherit from Package or from Packaged.
 *
 *  When modifications are made to a package they are not directly written,
 *  only when save() or saveAs() is called
 *
 *  To accomplish this modified files are first written to temporary files, when save() is called
 *  the temporary files are moved/copied.
 *
 *  Zip files are accessed using wxZip(Input|Output)Stream.
 *  The zip input stream appears to only allow one file at a time, since the stream itself maintains
 *  state about what file we are reading. 
 *  There are multiple solutions:
 *    1. (currently used) Open a new ZipInputStream for each file
 *    2. (may be faster) First read the file into a memory buffer,
 *      return a stream based on that buffer (StringInputStream).
 *
 *  TODO: maybe support sub packages (a package inside another package)?
 */
class Package : public IntrusivePtrVirtualBase {
  public:
	// --------------------------------------------------- : Managing the outside of the package
	
	/// Creates a new package
	Package();
	virtual ~Package();
	
	/// Is a file opened?
	bool isOpened() const;
	/// Must the package be saved with saveAs()?
	/// This is the case if the package has not been saved/opened before
	bool needSaveAs() const;
	/// Determines the short name of this package: the filename without path or extension
	String name() const;
	/// Return the relative filename of this file, the name and extension
	String relativeFilename() const;
	/// Return the absolute filename of this file
	const String& absoluteFilename() const;
	/// The time this package was last modified
	inline wxDateTime lastModified() const { return modified; }
		
	/// Open a package, should only be called when the package is constructed using the default constructor!
	/// @pre open not called before [TODO]
	void open(const String& package);
	
	/// Saves the package, by default saves as a zip file, unless
	/// it was already a directory
	/** If remove_unused=true all files that were in the file and
	 *  are not touched with referenceFile will be deleted from the new archive!
	 *  This is a form of garbage collection, to get rid of old picture files for example.
	 */
	void save(bool remove_unused = true);
	
	/// Saves the package under a different filename
	void saveAs(const String& package, bool remove_unused = true);
	
	
	// --------------------------------------------------- : Managing the inside of the package
	
	/// Open an input stream for a file in the package.
	InputStreamP openIn(const String& file);
	
	/// Open an output stream for a file in the package.
	/// (changes are only committed with save())
	OutputStreamP openOut(const String& file);
	
	/// Get a filename that can be written to to modfify a file in the package
	/// (changes are only committed with save())
	String nameOut(const String& file);
	
	/// Creates a new, unique, filename with the specified prefix and suffix
	/// for example newFileName("image/",".jpg") -> "image/1.jpg"
	/// Returns the name of a temporary file that can be written to.
	FileName newFileName(const String& prefix, const String& suffix);
	
	/// Signal that a file is still used by this package.
	/// Must be called for files not opened using openOut/nameOut
	/// If they are to be kept in the package.
	void referenceFile(const String& file);
	
	/// Get an 'absolute filename' for a file in the package.
	/// This file can later be opened from anywhere (other process) using openAbsoluteFile()
	String absoluteName(const String& file);
	
	/// Open a file given an absolute filename
	static InputStreamP openAbsoluteFile(const String& name);
	
	// --------------------------------------------------- : Managing the inside of the package : Reader/writer
	
	template <typename T>
	void readFile(const String& file, T& obj) {
		Reader reader(openIn(file), absoluteFilename() + _("/") + file);
		try {
			reader.handle_greedy(obj);
		} catch (const ParseError& err) {
			throw FileParseError(err.what(), absoluteFilename() + _("/") + file); // more detailed message
		}
	}
	template <typename T>
	T readFile(const String& file) {
		T obj;
		readFile(file, obj);
		return obj;
	}
	
	template <typename T>
	void writeFile(const String& file, const T& obj) {
		Writer writer(openOut(file));
		writer.handle(obj);
	}
	
	// --------------------------------------------------- : Private stuff
  private:
	
	/// Information about a file in the package
	struct FileInfo {
		FileInfo();
		~FileInfo();
		bool keep;               ///< Should this file be kept in the package? (as opposed to deleting it)
		String tempName;         ///< Name of the temporary file where new contents of this file are placed
		wxZipEntry* zipEntry;    ///< Entry in the zip file for this file
		inline bool wasWritten() ///< Is this file changed, and therefore written to a temporary file?
		 { return !tempName.empty(); }
	};
		
	/// Filename of the package
	String filename;
	/// Last modified time
	DateTime modified;
  public:
	/// Information on files in the package
	/** Note: must be public for DECLARE_TYPEOF to work */
	typedef map<String, FileInfo> FileInfos;
	inline const FileInfos& getFileInfos() const { return files; }
  private:
	/// All files in the package
	FileInfos files;
	/// Filestream for reading zip files
	wxFileInputStream* fileStream;
	/// Filestream for reading zip files
	wxZipInputStream*  zipStream;
	
	void openDirectory();
	void openSubdir(const String&);
	void openZipfile();
	void saveToDirectory(const String&, bool);
	void saveToZipfile(const String&, bool);
	FileInfos::iterator addFile(const String& file);
	static String toStandardName(const String& file);
};

// ----------------------------------------------------------------------------- : Packaged

/// Dependencies of a package
class PackageDependency : public IntrusivePtrBase<PackageDependency> {
  public:
	String  package;	///< Name of the package someone depends on
	Version version;	///< Minimal required version of that package
	
	DECLARE_REFLECTION();
};

/// Utility class for data types that are always stored in a package.
/** When the package is opened/saved a file describing the data object is read/written
 */
class Packaged : public Package {
  public:
	Packaged();
	virtual ~Packaged() {}
	
	Version version;		///< Version number of this package
	String short_name;		///< Short name of this package
	String full_name;		///< Name of this package, for menus etc.
	String icon_filename;	///< Filename of icon to use in package lists
	vector<PackageDependencyP> dependencies;	///< Dependencies of this package
	int    position_hint;	///< A hint for the package list
	
	/// Get an input stream for the package icon, if there is any
	InputStreamP openIconFile();
	
	/// Open a package, and read the data
	/** if just_header is true, then the package is not fully parsed.
	 */
	void open(const String& package, bool just_header = false);
	/// Ensure the package is fully loaded.
	void loadFully();
	void save();
	void saveAs(const String& package, bool remove_unused = true);
	
  protected:
	/// filename of the data file, and extension of the package file
	virtual String typeName() const = 0;
	/// Can be overloaded to do validation after loading
	virtual void validate(Version file_app_version);
	
	DECLARE_REFLECTION_VIRTUAL();
	
  private:
	bool   fully_loaded;	///< Is the package fully loaded?
	friend struct JustAsPackageProxy;
};

// ----------------------------------------------------------------------------- : EOF
#endif
