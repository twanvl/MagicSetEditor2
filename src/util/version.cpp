//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/version.hpp>
#include <util/reflect.hpp>

// ----------------------------------------------------------------------------- : Version

UInt Version::toNumber() const { return version; }

String Version::toString() const {
	return String() <<
		          ((version / 10000) % 100) <<
		_(".") << ((version / 100)   % 100) <<
		_(".") << ((version / 1)     % 100);
}

Version Version::fromString(const String& version) {
	UInt major = 0, minor = 0, build = 0;
	wxSscanf(version, _("%u.%u.%u"), &major, &minor, &build);
	return Version(major * 10000 + minor * 100 + build);
}


template <> void Reader::handle(Version& v) {
	v = Version::fromString(getValue());
}
template <> void Writer::handle(const Version& v) {
	handle(v.toString());
}
template <> void GetDefaultMember::handle(const Version& v) {
	handle(v.toNumber());
}

// ----------------------------------------------------------------------------- : Versions

// NOTE: Don't use leading zeroes, they mean octal
const Version app_version  = 301; // 0.3.1
const Char* version_suffix = _(" (beta)");

/*  Changes:
 *     0.2.0 : start of version numbering practice
 *     0.2.2 : _("include file")
 *     0.2.6 : fix in settings loading
 *     0.2.7 : new tag system, different style of close tags
 *     0.3.0 : port of code to C++
 *     0.3.1 : new keyword system, some new style options
 */
const Version file_version = 301; // 0.3.1
