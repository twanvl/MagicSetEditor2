//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_PACKAGE
#define HEADER_UTIL_IO_PACKAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/reflect.hpp>

class wxFileInputStream;
class wxZipInputStream;
class wxZipEntry;

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
class Package {
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
	/// Return the absolute filename of this file
	const String& absoluteFilename() const;
	
	/// Open a package, should only be called when the package is constructed using the default constructor!
	/// @pre open not called before [TODO]
	void open(const String& package);
	
	/// Saves the package, by default saves as a zip file, unless
	/// it was already a directory
	/** If removeUnused=true all files that were in the file and
	 *  are not touched with referenceFile will be deleted from the new archive!
	 *  This is a form of garbage collection, to get rid of old picture files for example.
	 */
	void save(bool removeUnused = true);
	
	/// Saves the package under a different filename
	void saveAs(const String& package, bool removeUnused = true);
	
	
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
	String newFileName(const String& prefix, const String& suffix);
	
	/// Signal that a file is still used by this package.
	/// Must be called for files not opened using openOut/nameOut
	/// If they are to be kept in the package.
	void referenceFile(const String& file);
	
	/// Get an 'absolute filename' for a file in the package.
	/// This file can later be opened from anywhere (other process) using openAbsoluteFile()
	String absoluteName(const String& file);
	
	/// Open a file given an absolute filename
	static InputStreamP openAbsoluteFile(const String& name);
	/*
	// --------------------------------------------------- : Managing the inside of the package : IO files
	
	template <typename T>
	void readFile<T> (String n, T& obj) {
		In i(openFileIn(n), filename + _("/") + n);
		try {
			i(obj);
		} catch (ParseError e) {
			throw FileParseError(e.what(), filename+_("/")+n); // more detailed message
		}
	}
	
	template <typename T>
	void writeFile(const String& file, T obj) {
	}
	*/
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

/// Utility class for data types that are always stored in a package.
/** When the package is opened/saved a file describing the data object is read/written
 */
class Packaged : public Package {
  public:
	Packaged();
	virtual ~Packaged() {}
	
	/// Open a package, and read the data
	void open(const String& package);
	void save();
	void saveAs(const String& package);
	
  protected:
	/// filename of the data file, and extension of the package file
	virtual String typeName() const = 0;
	/// Can be overloaded to do validation after loading
	virtual void validate() {}
	
	DECLARE_REFLECTION_VIRTUAL();
};

// ----------------------------------------------------------------------------- : EOF
#endif
