//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/settings.hpp>
class wxFileName;

// ----------------------------------------------------------------------------- : File names

/// Normalize a filename as much as possible, for real files
String normalize_filename(const String& filename);

/// Normalize a filename as much as possible, for files in packages
String normalize_internal_filename(const String& filename);

/// Should a file with the given name be ignored in packages?
/** true for hidden OS and version control files */
bool ignore_file(const String& name);

/// Add an extension to a filename if it is not already present
/**  add_extension("test",".txt") == "test.txt"
 *   add_extension("test.txt",".txt") == "test.txt"
 *   add_extension("test.xyz",".txt") == "test.xyz.txt"
 */
String add_extension(const String& filename, String const& extension);

/// Make sure a string is safe to use as a filename
String clean_filename(const String& name);

/// Change the filename fn if it already exists, in the way described by conflicts.
/** Returns true if the filename should be used, false if failed. */
bool resolve_filename_conflicts(wxFileName& fn, FilenameConflicts conflicts, set<String>& used);

// ----------------------------------------------------------------------------- : File info

/// Get the last modified time of a file
time_t file_modified_time(const String& name);

// ----------------------------------------------------------------------------- : Removing and renaming

bool create_directory(const String& path);

/// Ensure that the parent directories of the given filename exist
bool create_parent_dirs(const String& file);

/// Remove the given file
/** This is identical to wxRemoveFile, except that it doesn't show an error message if the file doesn't exist.
 *  (who thought that that was a good idea?)
 */
bool remove_file(const String& file);

/// Remove the given file or directory
/** It is not an error if the file doesn't exist.
 *  Removes all files in a directory.
 *  Returns true if the file is gone or was never there to begin with
 */
bool remove_file_or_dir(const String& file);

/// Rename a file or directory
bool rename_file_or_dir(const String& old_name, const String& new_name);

/// Move files/dirs that are ignored by packages to another directory
void move_ignored_files(const String& from_dir, const String& to_dir);

