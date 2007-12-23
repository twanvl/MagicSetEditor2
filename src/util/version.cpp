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
	if (version > 20000000) {
		// major > 2000, the version is a date, use ISO notation
		return String::Format(_("%04d-%02d-%02d"),
					(version / 10000)      ,
					(version / 100)   % 100,
					(version / 1)     % 100);
	} else {
		return String::Format(_("%d.%d.%d"),
					(version / 10000)      ,
					(version / 100)   % 100,
					(version / 1)     % 100);
	}
}

Version Version::fromString(const String& version) {
	UInt major = 0, minor = 0, build = 0;
	if (wxSscanf(version, _("%u.%u.%u"), &major, &minor, &build)<=1)  // a.b.c style
	    wxSscanf(version, _("%u-%u-%u"), &major, &minor, &build);  // date style
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
const Version app_version  = 306; // 0.3.6
#ifdef UNICODE
const Char* version_suffix = _(" (beta)");
#else
const Char* version_suffix = _(" (beta, ascii build)");
#endif

/*  Changes:
 *     0.2.0 : start of version numbering practice
 *     0.2.2 : _("include file")
 *     0.2.6 : fix in settings loading
 *     0.2.7 : new tag system, different style of close tags
 *     0.3.0 : port of code to C++
 *     0.3.1 : new keyword system, some new style options
 *     0.3.2 : package dependencies
 *     0.3.3 : keyword separator before/after
 *     0.3.4 : html export; choice rendering based on scripted 'image'
 *     0.3.5 : word lists, symbol font 'as text'
 *     0.3.6 : free rotation, rotation behaviour changed. 
 */
const Version file_version = 306; // 0.3.6
