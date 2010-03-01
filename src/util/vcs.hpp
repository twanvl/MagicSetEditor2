//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_VCS
#define HEADER_UTIL_IO_VCS

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/filename.h>

class VCS;

DECLARE_POINTER_TYPE(VCS);

template <>
void Reader::handle(VCSP& pointer);

// ----------------------------------------------------------------------------- : VCS

/// Interface to a version control system
/** This allows MSE to interact with various revision control systems directly
 *  rather than relying on the user to do so. Each method causes the external
 *  version control system to perform an operation on the specified file name.
 *  The default implementation just calls the normal file-handling functions.
 */
class VCS : public IntrusivePtrVirtualBase
{
  public:
	/// Add a file - it's assumed to already have been created
	virtual void addFile (const wxFileName& filename) {
	}
	/// Rename a file (currently unused)
	virtual void moveFile (const wxFileName& source, const wxFileName& destination) {
		wxRenameFile(source.GetFullName(), destination.GetFullName());
	}
	/// Delete a file right off the disk
	virtual void removeFile (const wxFileName& filename) {
		wxRemoveFile(filename.GetFullName());
	}
	
	DECLARE_REFLECTION_VIRTUAL();
};

// ----------------------------------------------------------------------------- : EOF
#endif
