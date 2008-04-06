//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_FILE_UTILS
#define HEADER_UTIL_FILE_UTILS

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : File names

/// Normalize a filename as much as possible, for real files
String normalize_filename(const String& filename);

/// Normalize a filename as much as possible, for files in packages
String normalize_internal_filename(const String& filename);

/// Should a file with the given name be ignored?
bool ignore_file(const String& name);

// ----------------------------------------------------------------------------- : Removing and renaming

/// Ensure that the parent directories of the given filename exist
bool create_parent_dirs(const String& file);

/// Remove the given file or directory
/** It is not an error if the file doesn't exist.
 *  Removes all files in a directory.
 *  Returns true if something was removed
 */
bool remove_file_or_dir(const String& file);

/// Rename a file or directory
bool rename_file_or_dir(const String& old_name, const String& new_name);

/// Move files/dirs that are ignored by packages to another directory
void move_ignored_files(const String& from_dir, const String& to_dir);

// ----------------------------------------------------------------------------- : EOF
#endif
