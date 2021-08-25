//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/reflect.hpp>
#include <util/dynamic_arg.hpp>
#include <util/error.hpp>
#include <util/file_utils.hpp>
#include <util/vcs.hpp>

class Package;
class wxFileInputStream;
class wxZipInputStream;
class wxZipEntry;
DECLARE_POINTER_TYPE(PackageDependency);

/// The package that is currently being written to
DECLARE_DYNAMIC_ARG(Package*, writing_package);
/// The package that is being put onto/read from the clipboard
DECLARE_DYNAMIC_ARG(Package*, clipboard_package);

// ----------------------------------------------------------------------------- : File names

/// A string standing for a filename in a package
/// Differs from a string because when writing the file should be included in the package
/// Behaviour is controled by dynamic args:
///   * clipboard_package
///   * writing_package
class LocalFileName {
public:
  LocalFileName() {}

  // Convert to a string that can be written to a package.
  // notifies the package that the underlying file is in use.
  // when writing to the clipboard, instead returns a global file reference.
  String toStringForWriting() const;
  // Construct a LocalFileName based on a string read from a package.
  // when reading from the clipboard, this will instead be a global file reference, and it is converted at this point.
  static LocalFileName fromReadString(const String&, const String& prefix = _("image"), String const& suffix = _(""));

  inline bool empty() const {
    return fn.empty();
  }
  inline bool operator == (LocalFileName const& that) const {
    return this->fn == that.fn;
  }

  inline String const& toStringForKey() const { return fn; }

private:
  LocalFileName(const wxString& fn) : fn(fn) {}
  String fn;
  friend class Package;
};

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

  VCSP  vcs;               ///< The version control system to use

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

  /// Open a package
  /**
   * Should only be called when the package is constructed using the default constructor!
   * 
   * If 'fast' is set, then for directories a full directory listing is not performed.
   * This means that the file_infos will not be fully initialized.
   * 
   * @pre open not called before [TODO]
   */
  void open(const String& package, bool fast = false);

  /// Saves the package
  /** 
   * By default saves as a zip file, unless it was already a directory.
   * 
   * If remove_unused=true all files that were in the file and
   *  are not touched with referenceFile will be deleted from the new archive!
   * This is a form of garbage collection, to get rid of old picture files for example.
   */
  void save(bool remove_unused = true);

  /// Saves the package under a different filename
  void saveAs(const String& package, bool remove_unused = true, bool as_directory = false);

  /// Saves the package under a different filename, but keep the old one open
  void saveCopy(const String& package);


  // --------------------------------------------------- : Managing the inside of the package

  /// Open an input stream for a file in the package.
  unique_ptr<wxInputStream> openIn(const String& file);
  inline unique_ptr<wxInputStream> openIn(const LocalFileName& file) {
    return openIn(file.fn);
  }

  /// Open an output stream for a file in the package.
  /// (changes are only committed with save())
  unique_ptr<wxOutputStream> openOut(const String& file);
  inline unique_ptr<wxOutputStream> openOut(const LocalFileName& file) {
    return openOut(file.fn);
  }

  /// Get a filename that can be written to to modfify a file in the package
  /// (changes are only committed with save())
  String nameOut(const String& file);
  inline String nameOut(const LocalFileName& file) {
    return nameOut(file.fn);
  }

  /// Creates a new, unique, filename with the specified prefix and suffix
  /// for example newFileName("image/",".jpg") -> "image/1.jpg"
  /// Returns the name of a temporary file that can be written to.
  LocalFileName newFileName(const String& prefix, const String& suffix);

  /// Signal that a file is still used by this package.
  /// Must be called for files not opened using openOut/nameOut
  /// If they are to be kept in the package.
  void referenceFile(const String& file);

  // --------------------------------------------------- : Managing the inside of the package : Reader/writer

  template <typename T>
  void readFile(const LocalFileName& file, T& obj);

  template <typename T>
  T readFile(const LocalFileName& file) {
    T obj;
    readFile(file, obj);
    return obj;
  }

  template <typename T>
  void writeFile(const String& file, const T& obj, Version file_version) {
    auto stream = openOut(file);
    Writer writer(*stream, file_version);
    writer.handle(obj);
  }

protected:
  virtual VCSP getVCS() {
      if (vcs == nullptr)
          return make_intrusive<VCS>();
      return vcs;
  }

  /// true if this is a zip file, false if a directory
  bool isZipfile() const { return !wxDirExists(filename); }

  // --------------------------------------------------- : Private stuff
  private:

  /// Information about a file in the package
  struct FileInfo {
    FileInfo();
    ~FileInfo();
    bool keep;               ///< Should this file be kept in the package? (as opposed to deleting it)
    bool created;            ///< Was this file just created (e.g. should the VCS add it?)
    String tempName;         ///< Name of the temporary file where new contents of this file are placed
    wxZipEntry* zipEntry;    ///< Entry in the zip file for this file
    /// Is this file changed, and therefore written to a temporary file?
    inline bool wasWritten() const { return !tempName.empty(); }
  };

  /// Filename of the package
  String filename;
  /// Last modified time
  DateTime modified;

public:
  /// Information on files in the package
  typedef map<String, FileInfo> FileInfos;
  inline const FileInfos& getFileInfos() const { return files; }
  /// When was a file last modified?
  DateTime modificationTime(const pair<String, FileInfo>& fi) const;
private:
  /// All files in the package
  FileInfos files;
  /// Filestream/zipstream for reading zip files
  unique_ptr<wxZipInputStream> zipStream;

  void loadZipStream();
  void openDirectory(bool fast = false);
  void openSubdir(const String&);
  void openZipfile();
  void reopen();
  void removeTempFiles(bool remove_unused);
  void clearKeepFlag();
  void saveToZipfile(const String&,   bool remove_unused, bool is_copy);
  void saveToDirectory(const String&, bool remove_unused, bool is_copy);
  FileInfos::iterator addFile(const String& file);

  /// Get an 'absolute filename' for a file in the package.
  /// This file can later be opened from anywhere (other process) using openAbsoluteFile()
  String absoluteName(const LocalFileName& file);

  /// Open a file given an absolute filename
  static unique_ptr<wxInputStream> openAbsoluteFile(const String& name);
  friend class LocalFileName;
};

// ----------------------------------------------------------------------------- : Packaged

/// Dependencies of a package
class PackageDependency : public IntrusivePtrBase<PackageDependency> {
public:
  String  package;         ///< Name of the package someone depends on
  vector<String> suggests; ///< Packages suggested to fulfill this dependency
  Version version;         ///< Minimal required version of that package

  DECLARE_REFLECTION();
};

/// Utility class for data types that are always stored in a package.
/** When the package is opened/saved a file describing the data object is read/written
 */
class Packaged : public Package {
public:
  Packaged();
  virtual ~Packaged() {}

  Version version;      ///< Version number of this package
  Version compatible_version;  ///< Earliest version number this package is compatible with
  String installer_group;    ///< Group to place this package in in the installer
  String short_name;      ///< Short name of this package
  String full_name;      ///< Name of this package, for menus etc.
  String icon_filename;    ///< Filename of icon to use in package lists
  vector<PackageDependencyP> dependencies;  ///< Dependencies of this package
  int    position_hint;    ///< A hint for the package list

  /// Get an input stream for the package icon, if there is any
  unique_ptr<wxInputStream> openIconFile();

  /// Open a package, and read the data
  /** if just_header is true, then the package is not fully parsed.
   */
  void open(const String& package, bool just_header = false);
  /// Ensure the package is fully loaded.
  void loadFully();
  void save();
  void saveAs(const String& package, bool remove_unused = true, bool as_directory = false);
  void saveCopy(const String& package);

  /// Check if this package lists a dependency on the given package
  /** This is done to force people to fill in the dependencies */
  void requireDependency(Packaged* package);

  inline bool isFullyLoaded () const {
    return fully_loaded;
  }

protected:
  /// filename of the data file, and extension of the package file
  virtual String typeName() const = 0;
  /// Can be overloaded to do validation after loading
  virtual void validate(Version file_app_version);
  /// What file version should be used for writing files?
  virtual Version fileVersion() const = 0;

  DECLARE_REFLECTION_VIRTUAL();
  friend void after_reading(Packaged& p, Version file_app_version);
  

private:
  bool   fully_loaded;  ///< Is the package fully loaded?
  friend struct JustAsPackageProxy;
  friend class Installer;
};

inline void after_reading(Packaged& p, Version file_app_version) {
  p.validate(file_app_version);
}

// ----------------------------------------------------------------------------- : IncludePackage

/// A package that just contains a bunch of files that are used from other packages
class IncludePackage : public Packaged {
protected:
  String typeName() const override;
  Version fileVersion() const override;
  DECLARE_REFLECTION_OVERRIDE();
};

// ----------------------------------------------------------------------------- : Utility

/// Open a package with the given filename
template <typename T>
intrusive_ptr<T> open_package(const String& filename) {
  intrusive_ptr<T> package(new T);
  package->open(filename);
  return package;
}

// ----------------------------------------------------------------------------- : readFile definition

// This is here because it uses dynamic_cast and must be to a complete type.
template <typename T>
inline void Package::readFile(const LocalFileName& file, T& obj) {
  auto stream = openIn(file);
  Reader reader(*stream, dynamic_cast<Packaged*>(this), absoluteFilename() + _("/") + file.fn);
  try {
    reader.handle_greedy(obj);
  } catch (const ParseError& err) {
    throw FileParseError(err.what(), absoluteFilename() + _("/") + file.fn); // more detailed message
  }
}

