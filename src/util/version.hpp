//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	
	bool operator < (Version v) { return version < v.versionSuffix; }
	
	/// Convert a version number to a string
	String toString();
	
	/// Convert a string to a version number
	static Version fromString(UInt version);
	
  private:
	UInt version; ///< Version number encoded as aabbcc, where a=major, b=minor, c=revision
};

// ----------------------------------------------------------------------------- : Versions

/// The verwsion number of MSE
const Version app_version  = 000300; // 0.3.0
const Char* version_suffix = _(" (beta)");

/// File version, usually the same as program version,
/** When no files are changed the file version is not incremented
 *  Changes:
 *     0.2.0 : start of version numbering practice
 *     0.2.2 : _("include file")
 *     0.2.6 : fix in settings loading
 *     0.2.7 : new tag system, different style of close tags
 *     0.3.0 : port of code to C++
 */
const Version file_version = 000300; // 0.3.0

// ----------------------------------------------------------------------------- : EOF
#endif
