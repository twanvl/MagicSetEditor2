//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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

bool is_filename_char(Char c) {
	return isAlnum(c) || c == _(' ') || c == _('_') || c == _('-') || c == _('.');
}

String clean_filename(const String& name) {
	String clean;
	FOR_EACH_CONST(c, name) {
		if (is_filename_char(c)) {
			clean += c;
		}
	}
	if (clean.empty() || starts_with(clean, _("."))) {
		clean = _("no-name") + clean;
	}
	return clean;
}

bool resolve_filename_conflicts(wxFileName& fn, FilenameConflicts conflicts, set<String>& used) {
	switch (conflicts) {
		case CONFLICT_KEEP_OLD:
			return !fn.FileExists();
		case CONFLICT_OVERWRITE:
			return true;
		case CONFLICT_NUMBER: {
			int i = 0;
			String ext = fn.GetExt();
			while(fn.FileExists()) {
				fn.SetExt(String() << ++i << _(".") << ext);
			}
			return true;
		}
		case CONFLICT_NUMBER_OVERWRITE: {
			int i = 0;
			String ext = fn.GetExt();
			while(used.find(fn.GetFullPath()) != used.end()) {
				fn.SetExt(String() << ++i << _(".") << ext);
			}
			return true;
		}
		default: {
			throw InternalError(_("resolve_filename_conflicts: default case"));
		}
	}
}

// ----------------------------------------------------------------------------- : Directories

bool create_parent_dirs(const String& file) {
	for (size_t pos = file.find_first_of(_("\\/"), 1) ;
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
		ok = true;
	}
	
	bool ok;
	
	void remove() {
		FOR_EACH_REVERSE(dir, to_delete) {
			if (!wxRmdir(dir)) {
				ok = false;
				handle_error(_("Cannot delete ") + dir + _("\n")
					_("The remainder of the package has still been removed, if possible.\n")
					_("Other packages may have been removed, including packages that this on is dependent on. Please remove manually."));
			}
		}
	}
	
	wxDirTraverseResult OnFile(const String& filename) {
		if (!wxRemoveFile(filename)) {
			ok = false;
			handle_error(_("Cannot delete ") + filename + _("\n")
				_("The remainder of the package has still been removed, if possible.\n")
				_("Other packages may have been removed, including packages that this on is dependent on. Please remove manually."));
		}
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
		RecursiveDeleter rd(name);
		{
			wxDir dir(name);
			dir.Traverse(rd);
		}
		rd.remove();
		return rd.ok;
	} else {
		return true;
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
