//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <util/io/package_manager.hpp>
#include <util/error.hpp>
#include <script/to_value.hpp> // for reflection
#include <script/profiler.hpp> // for PROFILER
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>
#include <util\vcs\git.hpp>
#include <util\vcs\git.hpp>
#include <util\vcs\subversion.hpp>

// ----------------------------------------------------------------------------- : Package : outside

IMPLEMENT_DYNAMIC_ARG(Package*, writing_package,   nullptr);
IMPLEMENT_DYNAMIC_ARG(Package*, clipboard_package, nullptr);

Package::Package()
  : zipStream (nullptr)
  , vcs (nullptr)
{}

Package::~Package() {
  // remove any remaining temporary files
  FOR_EACH(f, files) {
    if (f.second.wasWritten()) {
      remove_file(f.second.tempName);
    }
  }
}

bool Package::isOpened() const {
  return !filename.empty();
}
bool Package::needSaveAs() const {
  return filename.empty();
}
String Package::name() const {
  // wxFileName is too slow (profiled)
  //   wxFileName fn(filename);
  //   return fn.GetName();
  size_t ext   = filename.find_last_of(_('.'));
  size_t slash = filename.find_last_of(_("/\\:"), ext);
  if      (slash == String::npos && ext == String::npos) return filename;
  else if (slash == String::npos)                        return filename.substr(0,ext);
  else if (                         ext == String::npos) return filename.substr(slash+1);
  else                                                   return filename.substr(slash+1, ext-slash-1);
}
String Package::relativeFilename() const {
  size_t slash = filename.find_last_of(_("/\\:"));
  if (slash == String::npos) return filename;
  else                       return filename.substr(slash+1);
}
const String& Package::absoluteFilename() const {
  return filename;
}

void Package::open(const String& n, bool fast) {
  assert(!isOpened()); // not already opened
  PROFILER(_("open package"));
  // get absolute path
  wxFileName fn(n);
  fn.Normalize();
  filename = fn.GetFullPath();
  // get modified time
  if (!fn.FileExists() || !fn.GetTimes(0, &modified, 0)) {
    modified = wxDateTime(0.0); // long time ago
  }
  // type of package
  if (wxDirExists(filename)) {
    openDirectory(fast);
  } else if (wxFileExists(filename)) {
    openZipfile();
  } else {
    throw PackageNotFoundError(_("Package not found: '") + filename + _("'"));
  }
}

void Package::reopen() {
  if (wxDirExists(filename)) {
    // make sure we have no zip open
    zipStream.reset();
  } else {
    // reopen only needed for zipfile
    openZipfile();
  }
}


void Package::save(bool remove_unused) {
  assert(!needSaveAs());
  saveAs(filename, remove_unused);
}

void Package::saveAs(const String& name, bool remove_unused, bool as_directory) {
  // type of package
  if (wxDirExists(name) || as_directory) {
    saveToDirectory(name, remove_unused, false);
  } else {
    saveToZipfile  (name, remove_unused, false);
  }
  filename = name;
  removeTempFiles(remove_unused);
  reopen();
}

void Package::saveCopy(const String& name) {
  saveToZipfile(name, true, true);
  clearKeepFlag();
}

void Package::removeTempFiles(bool remove_unused) {
  // cleanup : remove temp files, remove deleted files from the list
  FileInfos::iterator it = files.begin();
  while (it != files.end()) {
    if (it->second.wasWritten()) {
      // remove corresponding temp file
      remove_file(it->second.tempName);
    }
    if (!it->second.keep && remove_unused) {
      // also remove the record of deleted files
      FileInfos::iterator to_remove = it;
      ++it;
      files.erase(to_remove);
    } else {
      // free zip entry, we will reopen the file
      it->second.keep = false;
      it->second.tempName.clear();
      delete it->second.zipEntry; 
      it->second.zipEntry = 0;
      ++it;
    }
  }
}

void Package::clearKeepFlag() {
  FOR_EACH(f, files) {
    f.second.keep = false;
  }
}

// ----------------------------------------------------------------------------- : Streams

/// Class to use as a superclass
class FileInputStream_aux {
protected:
  wxFileInputStream file_stream;
  inline FileInputStream_aux(const String& filename)
    : file_stream(filename)
  {}
};

/// Class that is a wxZipInputStream over a wxFileInput stream
/** Note that wxFileInputStream is also a base class, because it must be constructed first
 */
class ZipFileInputStream : private FileInputStream_aux, public wxZipInputStream {
public:
  ZipFileInputStream(const String& filename)
    : FileInputStream_aux(filename)
    , wxZipInputStream(file_stream)
  {}
  ZipFileInputStream(const String& filename, wxZipEntry* entry)
    : ZipFileInputStream(filename)
  {
    OpenEntry(*entry);
  }
};

/// A buffered version of wxFileInputStream
/** 2007-08-24:
 *    According to profiling this gives a significant speedup
 *    Bringing the avarage run time of read_utf8_line from 186k to 54k (in cpu time units)
 */
class BufferedFileInputStream : private FileInputStream_aux, public wxBufferedInputStream {
public:
  inline BufferedFileInputStream(const String& filename)
    : FileInputStream_aux(filename)
    , wxBufferedInputStream(file_stream)
  {}
};

// ----------------------------------------------------------------------------- : Package : inside

unique_ptr<wxInputStream> Package::openIn(const String& file) {
  if (!file.empty() && file.GetChar(0) == _('/')) {
    // absolute path, open file from another package
    Packaged* p = dynamic_cast<Packaged*>(this);
    return package_manager.openFileFromPackage(p, file).first;
  }
  FileInfos::iterator it = files.find(normalize_internal_filename(file));
  if (it == files.end()) {
    // does it look like a relative filename?
    if (filename.find(_(".mse-")) != String::npos) {
      throw PackageError(_ERROR_2_("file not found package like", file, filename));
    }
  }
  unique_ptr<wxInputStream> stream;
  if (it != files.end() && it->second.wasWritten()) {
    // written to this file, open the temp file
    stream = make_unique<wxFileInputStream>(it->second.tempName);
  } else if (wxFileExists(filename+_("/")+file)) {
    // a file in directory package
    stream = make_unique<wxFileInputStream>(filename+_("/")+file);
  } else if (wxFileExists(filename) && it != files.end() && it->second.zipEntry) {
    // a file in a zip archive
    stream = make_unique<ZipFileInputStream>(filename, it->second.zipEntry);
  } else {
    // shouldn't happen, packaged changed by someone else since opening it
    throw FileNotFoundError(file, filename);
  }
  if (!stream || !stream->IsOk()) {
    throw FileNotFoundError(file, filename);
  } else {
    return stream;
  }
}

unique_ptr<wxOutputStream> Package::openOut(const String& file) {
  return make_unique<wxFileOutputStream>(nameOut(file));
}

String Package::nameOut(const String& file) {
  assert(wxThread::IsMain()); // Writing should only be done from the main thread
  String name = normalize_internal_filename(file);
  FileInfos::iterator it = files.find(name);
  if (it == files.end()) {
    // new file
    it = addFile(name);
    it->second.created = true;
  }

  // return stream
  if (it->second.wasWritten()) {
    return it->second.tempName;
  } else {
    // create temp file
    String name = wxFileName::CreateTempFileName(_("mse"));
    it->second.tempName = name;
    return name;
  }
}

LocalFileName Package::newFileName(const String& prefix, const String& suffix) {
  assert(wxThread::IsMain()); // Writing should only be done from the main thread
  String name;
  UInt infix = 0;
  while (true) {
    // create filename
    name = prefix;
    name << ++infix;
    name += suffix;
    name = normalize_internal_filename(name);
    // check if a file with that name exists
    FileInfos::iterator it = files.find(name);
    if (it == files.end()) {
      // name doesn't exist yet
      it = addFile(name);
      it->second.created = true;
      return name;
    }
  }
}

void Package::referenceFile(const String& file) {
  if (file.empty()) return;
  FileInfos::iterator it = files.find(file);
  if (it == files.end()) throw InternalError(_("referencing a nonexistant file"));
  it->second.keep = true;
}

// ----------------------------------------------------------------------------- : LocalFileNames and absolute file references

String Package::absoluteName(const LocalFileName& file) {
  assert(wxThread::IsMain());
  FileInfos::iterator it = files.find(normalize_internal_filename(file.fn));
  if (it == files.end()) {
    throw FileNotFoundError(file.fn, filename);
  }
  if (it->second.wasWritten()) {
    // written to this file, return the temp file
    return it->second.tempName;
  } else if (wxFileExists(filename + _("/") + file.fn)) {
    // dir package
    return filename + _("/") + file.fn;
  } else {
    // assume zip package
    return filename + _("\1") + file.fn;
  }
}
// Open a file that is in some package
unique_ptr<wxInputStream> Package::openAbsoluteFile(const String& name) {
  size_t pos = name.find_first_of(_('\1'));
  if (pos == String::npos) {
    // temp or dir file
    auto stream = make_unique<wxFileInputStream>(name);
    if (!stream->IsOk()) throw FileNotFoundError(_("<unknown>"), name);
    return stream;
  } else {
    // packaged file, always in zip format
    Package p;
    p.open(name.substr(0, pos));
    return p.openIn( name.substr(pos + 1));
  }
}

String LocalFileName::toStringForWriting() const {
  if (!fn.empty() && clipboard_package()) {
    // use absolute names on clipboard
    try {
      return clipboard_package()->absoluteName(fn);
    } catch (const Error&) {
      // ignore errors
      return String();
    }
  } else if (!fn.empty() && writing_package()) {
    writing_package()->referenceFile(fn);
    return fn;
  } else {
    return fn;
  }
}

LocalFileName LocalFileName::fromReadString(String const& fn, String const& prefix, String const& suffix) {
  if (!fn.empty() && clipboard_package()) {
    // copy file into current package
    try {
      LocalFileName local_name = clipboard_package()->newFileName(_("image"),_("")); // a new unique name in the package, assume it's an image
      auto out_stream = clipboard_package()->openOut(local_name);
      auto in_stream  = Package::openAbsoluteFile(fn);
      out_stream->Write(*in_stream); // copy
      return local_name;
    } catch (const Error&) {
      // ignore errors
      return LocalFileName();
    }
  } else {
    return LocalFileName(fn);
  }
}


// ----------------------------------------------------------------------------- : Package : private

Package::FileInfo::FileInfo()
  : keep(false), created(false), zipEntry(nullptr)
{}

Package::FileInfo::~FileInfo() {
  delete zipEntry;
}

void Package::loadZipStream() {
  files.clear();
  while (true) {
    wxZipEntry* entry = zipStream->GetNextEntry();
    if (!entry) break;
    String name = normalize_internal_filename(entry->GetName(wxPATH_UNIX));
    files[name].zipEntry = entry;
  }
  zipStream->CloseEntry();
}

void Package::openDirectory(bool fast) {
  if (!fast) openSubdir(wxEmptyString);
}

void Package::openSubdir(const String& name) {
  wxDir d(filename + _("/") + name);
  if (!d.IsOpened()) return; // ignore errors here
  String f; // filename
  if (d.GetFirst(&f, _(".git"))) {
      queue_message(MESSAGE_INFO, filename + _(" under git"));
      vcs = make_intrusive<GitVCS>();
      vcs->pull(filename);
  }
  if (d.GetFirst(&f, _(".svn"))) {
      queue_message(MESSAGE_INFO, filename + _(" under subversion"));
      vcs = make_intrusive<SubversionVCS>();
      vcs->pull(filename);
  }
  // find files
  for(bool ok = d.GetFirst(&f, wxEmptyString, wxDIR_FILES | wxDIR_HIDDEN) ; ok ; ok = d.GetNext(&f)) {
    if (ignore_file(f)) continue;
    // add file to list of known files
    addFile(name + f);
    // get modified time
    wxDateTime file_time = file_modified_time(filename + _("/") + name + f);
    modified = max(modified,file_time);
  }
  // find subdirs
  for(bool ok = d.GetFirst(&f, wxEmptyString, wxDIR_DIRS | wxDIR_HIDDEN) ; ok ; ok = d.GetNext(&f)) {
    if (!f.empty() && f.GetChar(0) != _('.')) {
      // skip directories starting with '.', like ., .. and .svn
      openSubdir(name+f+_("/"));
    }
  }
}

void Package::openZipfile() {
  // open stream
  zipStream = make_unique<ZipFileInputStream>(filename);
  if (!zipStream->IsOk())  throw PackageError(_ERROR_1_("package not found", filename));
  // read zip entries
  loadZipStream();
}

void Package::saveToDirectory(const String& saveAs, bool remove_unused, bool is_copy) {
  // create directory?
  create_directory(saveAs);
  // write to a directory
  VCSP vcs = getVCS();
  FOR_EACH(f, files) {
    String f_out_path = saveAs + _("/") + f.first;
    if (!f.second.keep && remove_unused) {
      // remove files that are not to be kept
      // ignore failure (new file that is not kept)
      vcs->removeFile(f_out_path);
      continue;
    }
    // TODO: create subdirectory?
    // write file
    if (f.second.wasWritten()) {
      // move files that were updated
      remove_file(f_out_path);
      if (!(is_copy ? wxCopyFile  (f.second.tempName, f_out_path)
                    : wxRenameFile(f.second.tempName, f_out_path))) {
        throw PackageError(_ERROR_("unable to store file"));
      }
      if (f.second.created) {
        vcs->addFile(f_out_path);
        f.second.created = false;
      }
      else
      {
          vcs->updateFile(f_out_path);
      }
    } else if (filename != saveAs) {
      // save as, copy old filess
      if (isZipfile()) {
        auto in_stream  = openIn(f.first);
        wxFileOutputStream out(f_out_path);
        out.Write(*in_stream);
      } else {
        if (!wxCopyFile(filename+_("/")+f.first, f_out_path)) {
          throw PackageError(_ERROR_("unable to store file"));
        }
      }
      vcs->addFile(f_out_path);
    } else {
      // old file, just keep it
    }
  }
  vcs->commit(saveAs);
}

void Package::saveToZipfile(const String& saveAs, bool remove_unused, bool is_copy) {
  // create a temporary zip file name
  String tempFile = saveAs + _(".tmp");
  remove_file(tempFile);
  // open zip file
  try {
    unique_ptr<wxFileOutputStream> newFile(new wxFileOutputStream(tempFile));
    if (!newFile->IsOk()) throw PackageError(_ERROR_("unable to open output file"));
    unique_ptr<wxZipOutputStream>  newZip(new wxZipOutputStream(*newFile));
    if (!newZip->IsOk())  throw PackageError(_ERROR_("unable to open output file"));
    // copy everything to a new zip file, unless it's updated or removed
    if (zipStream) newZip->CopyArchiveMetaData(*zipStream);
    FOR_EACH(f, files) {
      if (!f.second.keep && remove_unused) {
        // to remove a file simply don't copy it
      } else if (!is_copy && f.second.zipEntry && !f.second.wasWritten()) {
        // old file, was also in zip, not changed
        // can't do this when saving a copy, since it destroys the zip entry
        zipStream->CloseEntry();
        newZip->CopyEntry(f.second.zipEntry, *zipStream);
        f.second.zipEntry = 0;
      } else {
        // changed file, or the old package was not a zipfile
        newZip->PutNextEntry(f.first);
        auto temp_stream = openIn(f.first);
        newZip->Write(*temp_stream);
      }
    }
    // close the old file
    if (!is_copy) {
      zipStream.reset();
    }
  } catch (Error const& e) {
    // when things go wrong delete the temp file
    remove_file(tempFile);
    throw e;
  }
  // replace the old file with the new file, in effect commiting the changes
  if (wxFileExists(saveAs)) {
    // rename old file to .bak
    remove_file(saveAs + _(".bak"));
    wxRenameFile(saveAs, saveAs + _(".bak"));
  }
  wxRenameFile(tempFile, saveAs);
  // re-open zip file
  filename = saveAs;
  openZipfile();
}


Package::FileInfos::iterator Package::addFile(const String& name) {
  return files.insert(make_pair(normalize_internal_filename(name), FileInfo())).first;
}

DateTime Package::modificationTime(const pair<String, FileInfo>& fi) const {
  if (fi.second.wasWritten()) {
    return wxFileName(fi.first).GetModificationTime();
  } else if (fi.second.zipEntry) {
    return fi.second.zipEntry->GetDateTime();
  } else if (wxFileExists(filename+_("/")+fi.first)) {
    return wxFileName(filename+_("/")+fi.first).GetModificationTime();
  } else {
    return DateTime((wxLongLong)0ul);
  }
}


// ----------------------------------------------------------------------------- : Packaged

template <> void Reader::handle(PackageDependency& dep) {
  if (!isCompound()) {
    handle(dep.package);
    size_t pos = dep.package.find_first_of(_(' '));
    if (pos != String::npos) {
      dep.version = Version::fromString(dep.package.substr(pos+1));
      dep.package = dep.package.substr(0,pos);
    }
  } else {
    handle(_("package"), dep.package);
    handle(_("suggests"), dep.suggests);
    handle(_("version"), dep.version);
  }
}
template <> void Writer::handle(const PackageDependency& dep) {
  if (dep.version != Version()) {
    handle(dep.package + _(" ") + dep.version.toString());
  } else {
    handle(dep.package);
  }
}

// note: reflection must be declared before it is used
IMPLEMENT_REFLECTION(Packaged) {
  REFLECT(short_name);
  REFLECT(full_name);
  REFLECT_N("icon", icon_filename);
  REFLECT_NO_SCRIPT(position_hint);
  REFLECT(installer_group);
  REFLECT(version);
  REFLECT(compatible_version);
  REFLECT_NO_SCRIPT_N("depends_ons", dependencies); // hack for singular_form
}

Packaged::Packaged()
  : position_hint(100000)
  , fully_loaded(true)
{}

unique_ptr<wxInputStream> Packaged::openIconFile() {
  if (!icon_filename.empty()) {
    return openIn(icon_filename);
  } else {
    return unique_ptr<wxInputStream>();
  }
}

// proxy object, that reads just the package header WITHOUT using the overloaded behaviour
struct JustAsPackageProxy {
  JustAsPackageProxy(Packaged* that) : that(that) {}
  Packaged* that;
};
template <> void Reader::handle(JustAsPackageProxy& object) {
  object.that->Packaged::reflect_impl(*this);
}

void Packaged::open(const String& package, bool just_header) {
  Package::open(package);
  fully_loaded = false;
  PROFILER(just_header ? _("open package header") : _("open package fully"));
  if (just_header) {
    // Read just the header (the part common to all Packageds)
    auto stream = openIn(typeName());
    Reader reader(*stream, this, absoluteFilename() + _("/") + typeName(), true);
    try {
      JustAsPackageProxy proxy(this);
      reader.handle_greedy(proxy);
    } catch (const ParseError& err) {
      throw FileParseError(err.what(), absoluteFilename() + _("/") + typeName()); // more detailed message
    }
  } else {
    loadFully();
  }
}

void Packaged::loadFully() {
  if (fully_loaded) return;
  auto stream = openIn(typeName());
  Reader reader(*stream, this, absoluteFilename() + _("/") + typeName());
  try {
    reader.handle_greedy(*this);
    fully_loaded = true; // only after loading and validating succeeded, be careful with recursion!
  } catch (const ParseError& err) {
    throw FileParseError(err.what(), absoluteFilename() + _("/") + typeName()); // more detailed message
  }
}

void Packaged::save() {
  WITH_DYNAMIC_ARG(writing_package, this);
  writeFile(typeName(), *this, fileVersion());
  referenceFile(typeName());
  Package::save();
}
void Packaged::saveAs(const String& package, bool remove_unused, bool as_directory) {
  WITH_DYNAMIC_ARG(writing_package, this);
  writeFile(typeName(), *this, fileVersion());
  referenceFile(typeName());
  Package::saveAs(package, remove_unused, as_directory);
}
void Packaged::saveCopy(const String& package) {
  WITH_DYNAMIC_ARG(writing_package, this);
  writeFile(typeName(), *this, fileVersion());
  referenceFile(typeName());
  Package::saveCopy(package);
}

void Packaged::validate(Version) {
  // a default for the short name
  if (short_name.empty()) {
    if (!full_name.empty()) short_name = full_name;
    short_name = name();
  }
  // check dependencies
  FOR_EACH(dep, dependencies) {
    package_manager.checkDependency(*dep, true);
  }
}

void Packaged::requireDependency(Packaged* package) {
  if (package == this) return; // dependency on self
  String n = package->relativeFilename();
  FOR_EACH(dep, dependencies) {
    if (dep->package == n) {
      if (package->version < dep->version) {
        queue_message(MESSAGE_WARNING,_ERROR_3_("package out of date", n, package->version.toString(), dep->version.toString()));
      } else if (package->compatible_version > dep->version) {
        queue_message(MESSAGE_WARNING,_ERROR_4_("package too new",     n, package->version.toString(), dep->version.toString(), relativeFilename()));
      } else {
        return; // ok
      }
    }
  }
  // dependency not found
  queue_message(MESSAGE_WARNING,_ERROR_4_("dependency not given", name(), package->relativeFilename(), package->relativeFilename(), package->version.toString()));
}

// ----------------------------------------------------------------------------- : IncludePackage

String IncludePackage::typeName() const { return _("include"); }
Version IncludePackage::fileVersion() const { return file_version_script; }

IMPLEMENT_REFLECTION(IncludePackage) {
  REFLECT_BASE(Packaged);
}
