//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_SUBVERSION
#define HEADER_UTIL_IO_SUBVERSION

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs.hpp>

// ----------------------------------------------------------------------------- : SubversionVCS

class SubversionVCS : public VCS {
  public:
	virtual void addFile (const wxFileName& filename);
	virtual void moveFile (const wxFileName& source, const wxFileName& destination);
	virtual void removeFile (const wxFileName& filename);
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
