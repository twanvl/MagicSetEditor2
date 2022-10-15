//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/error.hpp>
#include <data/settings.hpp>

class Game;
DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Card);

// ----------------------------------------------------------------------------- : FileFormat

/// A filter for a specific file format
class FileFormat : public IntrusivePtrVirtualBase {
public:
  virtual ~FileFormat() {}
  /// File extension used by this file format
  virtual String extension() = 0;
  /// What to match against
  virtual String matches() {
    return _("*.") + extension();
  }
  /// Name of the filter
  virtual String name() = 0;
  /// Can it be used for importing sets?
  virtual bool canImport() = 0;
  /// Can it be used for exporting sets for a particular game?
  virtual bool canExport(const Game&) = 0;
  /// Import using this filter
  virtual SetP importSet(const String& filename) {
    throw InternalError(_("Import not supported by this file format"));
  }
  /// Export using this filter
  /** If is_copy, then the set should not be modified */
  virtual void exportSet(Set& set, const String& filename, bool is_copy = false) {
    throw InternalError(_("Export not supported by this file format"));
  }
};

using FileFormatP = unique_ptr<FileFormat>;

// ----------------------------------------------------------------------------- : Formats

/// Initialize the list of file formats
/** Must be called before any other methods of this header */
void init_file_formats();

/// List of supported import formats
/** Formated as _("All supported (type1,...)|type1,...|name|type|...|All files(*.*)|*.*").
 *  For use in file selection dialogs.
 */
String import_formats();

// List of supported export formats that a set in a specific game can be exported.
/** Similair format as importFormats, except for 'all supported' and 'all files'
 */
String export_formats(const Game& game);

/// Opens a set with the specified filename.
/** File format is chosen based on the extension, default is fileFormats[0]
 *  (which is the MSE2 file filter)
 *  throws on error, always returns a valid set
 * 
 *  NOTE: String parameter must be passed by valueso we get a copy, otherwise
 *  changing the recent set list could change the filename while we are opening it
 *  (which would be bad)
 */
SetP import_set(String name);

/// Save a set under the specified name.
/** format_index specifies what format to use for saving, used as index in the list of file formats
 */
void export_set(Set& set, const String& filename, size_t format_index, bool is_copy = false);

// ----------------------------------------------------------------------------- : The formats

FileFormatP mse1_file_format();
FileFormatP mse2_file_format();
FileFormatP mtg_editor_file_format();

// ----------------------------------------------------------------------------- : Other ways to export

/// Export images for each card in a set to a list of files
void export_images(Window* parent, const SetP& set);

/// Export the image for each card in a list of cards
void export_images(const SetP& set, const vector<CardP>& cards,
                   const String& path, const String& filename_template, FilenameConflicts conflicts);

/// Export the image of a single card
void export_image(const SetP& set, const CardP& card, const String& filename);

/// Generate a bitmap image of a card
Bitmap export_bitmap(const SetP& set, const CardP& card, double zoom = 1.0);

/// Export a set to Magic Workstation format
void export_mws(Window* parent, const SetP& set);

/// Export a set to Apprentice
void export_apprentice(Window* parent, const SetP& set);

