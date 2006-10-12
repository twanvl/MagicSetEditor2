//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/package.hpp>
#include <util/error.hpp>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>
#include <boost/scoped_ptr.hpp>

DECLARE_TYPEOF(Package::FileInfos);

// ----------------------------------------------------------------------------- : Package : outside

Package::Package()
	: zipStream (nullptr)
	, fileStream(nullptr)
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
	return !filename.empty();
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
		throw PackageError(_("Package not found: '") + filename + _("'"));
	}
}

void Package::save(bool removeUnused) {
	assert(!needSaveAs());
	saveAs(filename, removeUnused);
}

void Package::saveAs(const String& name, bool removeUnused) {
	// type of package
	if (wxDirExists(name)) {
		saveToDirectory(name, removeUnused);
	} else {
		saveToZipfile  (name, removeUnused);
	}
	filename = name;
	// cleanup : remove temp files, remove deleted files from the list
	FileInfos::iterator it = files.begin();
	while (it != files.end()) {
		if (it->second.wasWritten()) {
			// remove corresponding temp file
			wxRemoveFile(it->second.tempName);
		}
		if (!it->second.keep && removeUnused) {
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

/// Class that is a wxZipInputStream over a wxFileInput stream
/** Note that wxFileInputStream is also a base class, because it must be constructed first
 *  This class requires a patch in wxWidgets (2.5.4)
 *   change zipstrm.cpp line 1745:;
 *        if ((!m_ffile || AtHeader()));
 *   to:
 *        if ((AtHeader()));
 *  It seems that in 2.6.3 this is no longer necessary (TODO: test)
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

InputStreamP Package::openIn(const String& file) {
//	if (!n.empty() && n.getChar(0) == _('/')) {
//		// absolute path, open file from another package
//		return packMan.openFileFromPackage(n);
//	}
	FileInfos::iterator it = files.find(toStandardName(file));
	if (it == files.end()) {
		throw FileNotFoundError(file, filename);
	}
	InputStreamP stream;
	if (it->second.wasWritten()) {
		// written to this file, open the temp file
		stream = new_shared1<wxFileInputStream>(it->second.tempName);
	} else if (wxFileExists(filename+_("/")+file)) {
		// a file in directory package
		stream = new_shared1<wxFileInputStream>(filename+_("/")+file);
	} else if (wxFileExists(filename) && it->second.zipEntry) {
		// a file in a zip archive
		stream = static_pointer_cast<wxZipInputStream>(
					new_shared2<ZipFileInputStream>(filename, it->second.zipEntry));
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

String Package::newFileName(const String& prefix, const String& suffix) {
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

String Package::absoluteName(const String& file) {
	assert(wxThread::IsMain());
	FileInfos::iterator it = files.find(toStandardName(file));
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

// ----------------------------------------------------------------------------- : Package : private

Package::FileInfo::FileInfo()
	: keep(false), zipEntry(nullptr)
{}

Package::FileInfo::~FileInfo() {
	delete zipEntry;
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
		// add file
		addFile(name + f);
		// get modified time
		wxFileName fn(filename + _("/") + name + f);
		wxDateTime mod;
		if (fn.GetTimes(0, &mod, 0) && mod > modified) modified = mod;
	}
	// find subdirs
	for(bool ok = d.GetFirst(&f, wxEmptyString, wxDIR_DIRS | wxDIR_HIDDEN) ; ok ; ok = d.GetNext(&f)) {
		openSubdir(name+f+_("/"));
	}
}

void Package::openZipfile() {
	// close old streams
	delete fileStream; fileStream = nullptr;
	delete zipStream;  zipStream  = nullptr;
	// open streams
	fileStream = new wxFileInputStream(filename);
	if (!fileStream->IsOk()) throw PackageError(_("Package not found: '")+filename+_("'"));
	zipStream  = new wxZipInputStream(*fileStream);
	if (!zipStream->IsOk()) throw PackageError(_("Package not found: '")+filename+_("'"));
	// read zip entries
	while (true) {
		wxZipEntry* entry = zipStream->GetNextEntry();
		if (!entry) break;
		String name = toStandardName(entry->GetName());
		files[name].zipEntry = entry;
	}
	zipStream->CloseEntry();
}


void Package::saveToDirectory(const String& saveAs, bool removeUnused) {
	// write to a directory
	FOR_EACH(f, files) {
		if (!f.second.keep && removeUnused) {
			// remove files that are not to be kept
			// ignore failure (new file that is not kept)
			wxRemoveFile(saveAs+_("/")+f.first);
		} else if (f.second.wasWritten()) {
			// move files that were updated
			wxRemoveFile(saveAs+_("/")+f.first);
			if (!wxRenameFile(f.second.tempName, saveAs+_("/")+f.first)) {
				throw PackageError(_("Error while saving, unable to store file"));
			}
		} else if (filename != saveAs) {
			// save as, copy old filess
			if (!wxCopyFile(filename+_("/")+f.first, saveAs+_("/")+f.first)) {
				throw PackageError(_("Error while saving, unable to store file"));
			}
		} else {
			// old file, just keep it
		}
	}
}

void Package::saveToZipfile(const String& saveAs, bool removeUnused) {
	// create a temporary zip file name
	String tempFile = saveAs + _(".tmp");
	wxRemoveFile(tempFile);
	// open zip file
	try {
		scoped_ptr<wxFileOutputStream> newFile(new wxFileOutputStream(tempFile));
		if (!newFile->IsOk()) throw PackageError(_("Error while saving, unable to open output file"));
		scoped_ptr<wxZipOutputStream>  newZip(new wxZipOutputStream(*newFile));
		if (!newZip->IsOk())  throw PackageError(_("Error while saving, unable to open output file"));
		// copy everything to a new zip file, unless it's updated or removed
		if (zipStream) newZip->CopyArchiveMetaData(*zipStream);
		FOR_EACH(f, files) {
			if (!f.second.keep && removeUnused) {
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
	return files.insert(make_pair(toStandardName(name), FileInfo())).first;
}

String Package::toStandardName(const String& name) {
	String ret;
	FOR_EACH_CONST(c, name) {
		if (c==_('\\')) ret += _('/');
		else            ret += toLower(c);
	}
	return ret;
}

// ----------------------------------------------------------------------------- : Packaged

// note: reflection must be declared before it is used
IMPLEMENT_REFLECTION(Packaged) {
	// default does nothing
}

Packaged::Packaged() {
}

void Packaged::open(const String& package) {
	Package::open(package);
	Reader reader(openIn(typeName()), absoluteFilename() + _("/") + typeName());
	try {
		reader.handle(*this);
	} catch (const ParseError& err) {
		throw FileParseError(err.what(), absoluteFilename() + _("/") + typeName()); // more detailed message
	}
}
void Packaged::save() {
	//writeFile(thisT().fileName, thisT());
	Package::save();
}
void Packaged::saveAs(const String& package) {
	//writeFile(thisT().fileName, thisT());
	Package::saveAs(package);
}
