//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <util/io/package_manager.hpp>
#include <util/error.hpp>
#include <script/to_value.hpp> // for reflection
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>
#include <boost/scoped_ptr.hpp>

DECLARE_TYPEOF(Package::FileInfos);
DECLARE_TYPEOF_COLLECTION(PackageDependencyP);

// ----------------------------------------------------------------------------- : Package : outside

IMPLEMENT_DYNAMIC_ARG(Package*, writing_package,   nullptr);
IMPLEMENT_DYNAMIC_ARG(Package*, clipboard_package, nullptr);

Package::Package()
	: fileStream(nullptr)
	, zipStream (nullptr)
{}

Package::~Package() {
	delete zipStream;
	delete fileStream;
	// remove any remaining temporary files
	FOR_EACH(f, files) {
		if (f.second.wasWritten()) {
			wxRemoveFile(f.second.tempName);
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

void Package::open(const String& n) {
	assert(!isOpened()); // not already opened
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
		openDirectory();
	} else if (wxFileExists(filename)) {
		openZipfile();
	} else {
		throw PackageNotFoundError(_("Package not found: '") + filename + _("'"));
	}
}

void Package::save(bool remove_unused) {
	assert(!needSaveAs());
	saveAs(filename, remove_unused);
}

void Package::saveAs(const String& name, bool remove_unused) {
	// type of package
	if (wxDirExists(name)) {
		saveToDirectory(name, remove_unused);
	} else {
		saveToZipfile  (name, remove_unused);
	}
	filename = name;
	// cleanup : remove temp files, remove deleted files from the list
	FileInfos::iterator it = files.begin();
	while (it != files.end()) {
		if (it->second.wasWritten()) {
			// remove corresponding temp file
			wxRemoveFile(it->second.tempName);
		}
		if (!it->second.keep && remove_unused) {
			// also remove the record of deleted files
			FileInfos::iterator toRemove = it;
			++it;
			files.erase(toRemove);
		} else {
			// free zip entry, we will reopen the file
			it->second.keep = false;
			it->second.tempName.clear();
			delete it->second.zipEntry; 
			it->second.zipEntry = 0;
			++it;
		}
	}
	// reopen only needed for zipfile
	if (!wxDirExists(name)) {
		openZipfile();
	} else {
		// make sure we have no zip open
		delete zipStream;  zipStream  = nullptr;
		delete fileStream; fileStream = nullptr;
	}
}

// ----------------------------------------------------------------------------- : Package : inside

#if 0
/// Class that is a wxZipInputStream over a wxFileInput stream
/** Note that wxFileInputStream is also a base class, because it must be constructed first
 *  This class requires a patch in wxWidgets (2.5.4)
 *   change zipstrm.cpp line 1745:;
 *        if ((!m_ffile || AtHeader()));
 *   to:
 *        if ((AtHeader()));
 *  It seems that in 2.6.3 this is no longer necessary (TODO: test)
 *
 *  NOTE: Not used with wx 2.6.3, since it doesn't support seeking
 */
class ZipFileInputStream : private wxFileInputStream, public wxZipInputStream {
  public:
	ZipFileInputStream(const String& filename, wxZipEntry* entry)
		: wxFileInputStream(filename)
		, wxZipInputStream(static_cast<wxFileInputStream&>(*this))
	{
		OpenEntry(*entry);
	}
};
#endif

class BufferedFileInputStream_aux {
  protected:
	wxFileInputStream file_stream;
	inline BufferedFileInputStream_aux(const String& filename)
		: file_stream(filename)
	{}
};
/// A buffered version of wxFileInputStream
/** 2007-08-24:
 *    According to profiling this gives a significant speedup
 *    Bringing the avarage run time of read_utf8_line from 186k to 54k (in cpu time units)
 */
class BufferedFileInputStream : private BufferedFileInputStream_aux, public wxBufferedInputStream {
  public:
	inline BufferedFileInputStream(const String& filename)
		: BufferedFileInputStream_aux(filename)
		, wxBufferedInputStream(file_stream)
	{}
};

InputStreamP Package::openIn(const String& file) {
	if (!file.empty() && file.GetChar(0) == _('/')) {
		// absolute path, open file from another package
		Packaged* p = dynamic_cast<Packaged*>(this);
		return packages.openFileFromPackage(p, file);
	}
	FileInfos::iterator it = files.find(normalize_internal_filename(file));
	if (it == files.end()) {
		// does it look like a relative filename?
		if (filename.find(_(".mse-")) != String::npos) {
			throw PackageError(_ERROR_2_("file not found package like", file, filename));
		}
		throw FileNotFoundError(file, filename);
	}
	InputStreamP stream;
	if (it->second.wasWritten()) {
		// written to this file, open the temp file
		stream = new_shared1<BufferedFileInputStream>(it->second.tempName);
	} else if (wxFileExists(filename+_("/")+file)) {
		// a file in directory package
		stream = new_shared1<BufferedFileInputStream>(filename+_("/")+file);
	} else if (wxFileExists(filename) && it->second.zipEntry) {
		// a file in a zip archive
		// somebody in wx thought seeking was no longer needed, it now only works with the 'compatability constructor'
		stream = new_shared2<wxZipInputStream>(filename, it->second.zipEntry->GetName());
		//stream = static_pointer_cast<wxZipInputStream>(
		//			new_shared2<ZipFileInputStream>(filename, it->second.zipEntry));
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

OutputStreamP Package::openOut(const String& file) {
	return new_shared1<wxFileOutputStream>(nameOut(file));
}

String Package::nameOut(const String& file) {
	assert(wxThread::IsMain()); // Writing should only be done from the main thread
	FileInfos::iterator it = files.find(file);
	if (it == files.end()) {
		// new file
		it = addFile(file);
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

FileName Package::newFileName(const String& prefix, const String& suffix) {
	assert(wxThread::IsMain()); // Writing should only be done from the main thread
	String name;
	UInt infix = 0;
	while (true) {
		// create filename
		name = prefix;
		name << ++infix;
		name += suffix;
		// check if a file with that name exists
		FileInfos::iterator it = files.find(name);
		if (it == files.end()) {
			// name doesn't exist yet
			addFile(name);
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

String Package::absoluteName(const String& file) {
	assert(wxThread::IsMain());
	FileInfos::iterator it = files.find(normalize_internal_filename(file));
	if (it == files.end()) {
		throw FileNotFoundError(file, filename);
	}
	if (it->second.wasWritten()) {
		// written to this file, return the temp file
		return it->second.tempName;
	} else if (wxFileExists(filename+_("/")+file)) {
		// dir package
		return filename+_("/")+file;
	} else {
		// assume zip package
		return filename+_("\1")+file;
	}
}
// Open a file that is in some package
InputStreamP Package::openAbsoluteFile(const String& name) {
	size_t pos = name.find_first_of(_('\1'));
	if (pos == String::npos) {
		// temp or dir file
		shared_ptr<wxFileInputStream> f = new_shared1<wxFileInputStream>(name);
		if (!f->IsOk()) throw FileNotFoundError(_("<unknown>"), name);
		return f;
	} else {
		// packaged file, always in zip format
		Package p;
		p.open(name.substr(0, pos));
		return p.openIn( name.substr(pos + 1));
	}
}

// ----------------------------------------------------------------------------- : Package : private

Package::FileInfo::FileInfo()
	: keep(false), zipEntry(nullptr)
{}

Package::FileInfo::~FileInfo() {
	delete zipEntry;
}

void Package::loadZipStream() {
	while (true) {
		wxZipEntry* entry = zipStream->GetNextEntry();
		if (!entry) break;
		String name = normalize_internal_filename(entry->GetName());
		files[name].zipEntry = entry;
	}
	zipStream->CloseEntry();
}

void Package::openDirectory() {
	openSubdir(wxEmptyString);
}

void Package::openSubdir(const String& name) {
	wxDir d(filename + _("/") + name);
	if (!d.IsOpened()) return; // ignore errors here
	// find files
	String f; // filename
	for(bool ok = d.GetFirst(&f, wxEmptyString, wxDIR_FILES | wxDIR_HIDDEN) ; ok ; ok = d.GetNext(&f)) {
		if (ignore_file(f)) continue;
		// add file
		addFile(name + f);
		// get modified time
		wxFileName fn(filename + _("/") + name + f);
		wxDateTime mod;
		if (fn.GetTimes(0, &mod, 0) && mod > modified) modified = mod;
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
	// close old streams
	delete fileStream; fileStream = nullptr;
	delete zipStream;  zipStream  = nullptr;
	// open streams
	fileStream = new wxFileInputStream(filename);
	if (!fileStream->IsOk()) throw PackageError(_ERROR_1_("package not found", filename));
	zipStream  = new wxZipInputStream(*fileStream);
	if (!zipStream->IsOk())  throw PackageError(_ERROR_1_("package not found", filename));
	// read zip entries
	loadZipStream();
}

void Package::saveToDirectory(const String& saveAs, bool remove_unused) {
	// write to a directory
	FOR_EACH(f, files) {
		if (!f.second.keep && remove_unused) {
			// remove files that are not to be kept
			// ignore failure (new file that is not kept)
			wxRemoveFile(saveAs+_("/")+f.first);
		} else if (f.second.wasWritten()) {
			// move files that were updated
			wxRemoveFile(saveAs+_("/")+f.first);
			if (!wxRenameFile(f.second.tempName, saveAs+_("/")+f.first)) {
				throw PackageError(_ERROR_("unable to store file"));
			}
		} else if (filename != saveAs) {
			// save as, copy old filess
			if (!wxCopyFile(filename+_("/")+f.first, saveAs+_("/")+f.first)) {
				throw PackageError(_ERROR_("unable to store file"));
			}
		} else {
			// old file, just keep it
		}
	}
}

void Package::saveToZipfile(const String& saveAs, bool remove_unused) {
	// create a temporary zip file name
	String tempFile = saveAs + _(".tmp");
	wxRemoveFile(tempFile);
	// open zip file
	try {
		scoped_ptr<wxFileOutputStream> newFile(new wxFileOutputStream(tempFile));
		if (!newFile->IsOk()) throw PackageError(_ERROR_("unable to open output file"));
		scoped_ptr<wxZipOutputStream>  newZip(new wxZipOutputStream(*newFile));
		if (!newZip->IsOk())  throw PackageError(_ERROR_("unable to open output file"));
		// copy everything to a new zip file, unless it's updated or removed
		if (zipStream) newZip->CopyArchiveMetaData(*zipStream);
		FOR_EACH(f, files) {
			if (!f.second.keep && remove_unused) {
				// to remove a file simply don't copy it
			} else if (f.second.zipEntry && !f.second.wasWritten()) {
				// old file, was also in zip, not changed
				zipStream->CloseEntry();
				newZip->CopyEntry(f.second.zipEntry, *zipStream);
				f.second.zipEntry = 0;
			} else {
				// changed file, or the old package was not a zipfile
				newZip->PutNextEntry(f.first);
				InputStreamP temp = openIn(f.first);
				newZip->Write(*temp);
			}
		}
		// close the old file
		delete zipStream;  zipStream  = nullptr;
		delete fileStream; fileStream = nullptr;
	} catch (Error e) {
		// when things go wrong delete the temp file
		wxRemoveFile(tempFile);
		throw e;
	}
	// replace the old file with the new file, in effect commiting the changes
	if (wxFileExists(saveAs)) {
		// rename old file to .bak
		wxRemoveFile(saveAs + _(".bak"));
		wxRenameFile(saveAs, saveAs + _(".bak"));
	}
	wxRenameFile(tempFile, saveAs);
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
		return DateTime();
	}
}


// ----------------------------------------------------------------------------- : Packaged

template <> void Reader::handle(PackageDependency& dep) {
	if (!isComplex()) {
		handle(dep.package);
		size_t pos = dep.package.find_first_of(_(' '));
		if (pos != String::npos) {
			dep.version = Version::fromString(dep.package.substr(pos+1));
			dep.package = dep.package.substr(0,pos);
		}
	} else {
		handle(_("package"), dep.package);
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
	REFLECT_NO_SCRIPT_N("depends ons", dependencies); // hack for singular_form
}

Packaged::Packaged()
	: position_hint(100000)
	, fully_loaded(true)
{}

InputStreamP Packaged::openIconFile() {
	if (!icon_filename.empty()) {
		return openIn(icon_filename);
	} else {
		return InputStreamP();
	}
}

// proxy object, that reads just WITHOUT using the overloaded behaviour
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
	if (just_header) {
		// Read just the header (the part common to all Packageds)
		Reader reader(openIn(typeName()), this, absoluteFilename() + _("/") + typeName(), true);
		try {
			JustAsPackageProxy proxy(this);
			reader.handle_greedy(proxy);
			Packaged::validate(reader.file_app_version);
		} catch (const ParseError& err) {
			throw FileParseError(err.what(), absoluteFilename() + _("/") + typeName()); // more detailed message
		}
	} else {
		loadFully();
	}
}
void Packaged::loadFully() {
	if (fully_loaded) return;
	fully_loaded = true;
	Reader reader(openIn(typeName()), this, absoluteFilename() + _("/") + typeName());
	try {
		reader.handle_greedy(*this);
		validate(reader.file_app_version);
	} catch (const ParseError& err) {
		throw FileParseError(err.what(), absoluteFilename() + _("/") + typeName()); // more detailed message
	}
}

void Packaged::save() {
	WITH_DYNAMIC_ARG(writing_package, this);
	writeFile(typeName(), *this);
	referenceFile(typeName());
	Package::save();
}
void Packaged::saveAs(const String& package, bool remove_unused) {
	WITH_DYNAMIC_ARG(writing_package, this);
	writeFile(typeName(), *this);
	referenceFile(typeName());
	Package::saveAs(package, remove_unused);
}

void Packaged::validate(Version) {
	// a default for the short name
	if (short_name.empty()) {
		if (!full_name.empty()) short_name = full_name;
		short_name = name();
	}
	// check dependencies
	FOR_EACH(dep, dependencies) {
		packages.checkDependency(*dep, true);
	}
}

void Packaged::requireDependency(Packaged* package) {
	if (package == this) return; // dependency on self
	String n = package->relativeFilename();
	FOR_EACH(dep, dependencies) {
		if (dep->package == n) {
			if (package->version < dep->version) {
				handle_warning(_ERROR_3_("package out of date", n, package->version.toString(), dep->version.toString()), false);
			} else if (package->compatible_version > dep->version) {
				handle_warning(_ERROR_4_("package too new",     n, package->version.toString(), dep->version.toString(), relativeFilename()), false);
			} else {
				return; // ok
			}
		}
	}
	// dependency not found
	handle_warning(_ERROR_4_("dependency not given", name(), package->relativeFilename(), package->relativeFilename(), package->version.toString()), false);
}

// ----------------------------------------------------------------------------- : IncludePackage

String IncludePackage::typeName() const { return _("include"); }

IMPLEMENT_REFLECTION(IncludePackage) {
	REFLECT_BASE(Packaged);
}
