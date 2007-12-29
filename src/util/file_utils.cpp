//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/file_utils.hpp>
#include <wx/filename.h>
#include <wx/dir.h>

DECLARE_TYPEOF_COLLECTION(String);

// ----------------------------------------------------------------------------- : File names

String normalize_filename(const String& name) {
	wxFileName fn(name);
	fn.Normalize();
	return fn.GetFullPath();
}

String normalize_internal_filename(const String& name) {
	String ret;
	FOR_EACH_CONST(c, name) {
		if (c==_('\\')) ret += _('/');
		else            ret += toLower(c);
	}
	return ret;
}

bool ignore_file(const String& name) {
	// Files that are never part of a package,
	// i.e. random stuff the OS file manager dumps without being asked
	return name == _("Thumbs.db"); // winXP explorer thumbnails
}

// ----------------------------------------------------------------------------- : Directories

bool create_parent_dirs(const String& file) {
	for (size_t pos = file.find_first_of(_("\\/")) ;
	     pos != String::npos ;
	     pos = file.find_first_of(_("\\/"),pos+1)) {
		String part = file.substr(0,pos);
		if (!wxDirExists(part)) {
			if (!wxMkdir(part)) return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------------- : Removing

class RecursiveDeleter : public wxDirTraverser {
  public:
	RecursiveDeleter(const String& start) {
		to_delete.push_back(start);
	}
	~RecursiveDeleter() {
		FOR_EACH_REVERSE(dir, to_delete) {
			wxRmdir(dir);
		}
	}
	
	wxDirTraverseResult OnFile(const String& filename) {
		if (!wxRemoveFile(filename))
			handle_error(_("Cannot delete ") + filename + _(". ")
				_("The remainder of the package has still been removed, if possible.")
				_("Other packages may have been removed, including packages that this on is dependent on. Please remove manually."));
		return wxDIR_CONTINUE;
	}
	wxDirTraverseResult OnDir(const String& dirname) {
		to_delete.push_back(dirname);
		return wxDIR_CONTINUE;
	}
  private:
	vector<String> to_delete;
};

bool remove_file_or_dir(const String& name) {
	if (wxFileExists(name)) {
		return wxRemoveFile(name);
	} else if (wxDirExists(name)) {
		wxDir dir(name);
		RecursiveDeleter rd(name);
		dir.Traverse(rd);
		return true;
	} else {
		return false;
	}
}

// ----------------------------------------------------------------------------- : Renaming

bool rename_file_or_dir(const String& from, const String& to) {
	create_parent_dirs(to);
	return wxRenameFile(from, to);
}

// ----------------------------------------------------------------------------- : Moving

class IgnoredMover : public wxDirTraverser {
  public:
	IgnoredMover(const String& from, const String& to)
		: from(from), to(to)
	{}
	wxDirTraverseResult OnFile(const String& filename) {
		tryMove(filename);
		return wxDIR_CONTINUE;
	}
	wxDirTraverseResult OnDir(const String& dirname) {
		return tryMove(dirname) ? wxDIR_IGNORE : wxDIR_CONTINUE;
	}
  private:
	String from, to;
	bool tryMove(const String& from_path) {
		if (is_substr(from_path,0,from)) {
			String to_path = to + from_path.substr(from.size());
			return rename_file_or_dir(from_path, to_path);
		} else {
			// This shouldn't happen
			return false;
		}
	}
};

void move_ignored_files(const String& from_dir, const String& to_dir) {
	if (wxDirExists(from_dir) && wxDirExists(to_dir)) {
		wxDir dir(from_dir);
		IgnoredMover im(from_dir, to_dir);
		dir.Traverse(im);
	}
}
