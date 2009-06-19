//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_VERSION
#define HEADER_UTIL_VERSION

/** @file util/version.hpp
 *
 *  @brief Utility functions related to version numbers.
 *  This header also stores the MSE version number.
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Version datatype

/// A version number
struct Version {
  public:
	Version()             : version(0)       {}
	Version(UInt version) : version(version) {}
	
	inline bool operator == (Version v) const { return version == v.version; }
	inline bool operator != (Version v) const { return version != v.version; }
	inline bool operator <  (Version v) const { return version <  v.version; }
	inline bool operator <= (Version v) const { return version <= v.version; }
	inline bool operator >  (Version v) const { return version >  v.version; }
	inline bool operator >= (Version v) const { return version >= v.version; }
	
	/// Convert a version number to a string
	String toString() const;
	/// Get the version number as an integer number
	UInt   toNumber() const;
	
	/// Convert a string to a version number
	static Version fromString(const String& version);
	
  private:
	UInt version; ///< Version number encoded as aabbcc, where a=major, b=minor, c=revision
};

// ----------------------------------------------------------------------------- : Versions

/// The version number of MSE
extern const Version app_version;
extern const Char* version_suffix;

/// Which version of MSE are the files we write out compatible with?
/** When no files are changed the file version is not incremented
 */
extern const Version file_version_locale;
extern const Version file_version_set;
extern const Version file_version_game;
extern const Version file_version_stylesheet;
extern const Version file_version_symbol_font;
extern const Version file_version_export_template;
extern const Version file_version_installer;
extern const Version file_version_symbol;
extern const Version file_version_clipboard;
extern const Version file_version_script;

// ----------------------------------------------------------------------------- : EOF
#endif
