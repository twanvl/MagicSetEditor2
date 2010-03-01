//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs/subversion.hpp>

// ----------------------------------------------------------------------------- : SVN File Manipulation

void SubversionVCS::addFile(const wxFileName& filename)
{
	String name = filename.GetFullPath();
	const Char* name_c[] = {_("svn"), _("add"), name.c_str(), nullptr};
	switch (wxExecute(const_cast<Char**>(name_c), wxEXEC_SYNC)) // Yuck, const_cast
	{
		// Success
		case 0:
			return;
		// Couldn't run SVN
		case -1:
			handle_error(String(_("Can't run SVN.")));
			VCS::addFile(filename);
			return;
		// SVN error
		default:
			handle_error(String(_("SVN encountered an error")));
			VCS::addFile(filename);
			return;
	}
}

void SubversionVCS::moveFile(const wxFileName& source, const wxFileName& dest)
{
	String source_name = source.GetFullPath(), dest_name = dest.GetFullPath();
	const Char* name_c[] = {_("svn"), _("mv"), source_name.c_str(), dest_name.c_str(), nullptr};
	switch (wxExecute(const_cast<Char**>(name_c), wxEXEC_SYNC)) // Once again, yuck
	{
		// Success
		case 0:
			return;
		// Couldn't run SVN
		case -1:
			handle_error(String(_("Can't run SVN.")));
			VCS::moveFile(source, dest);
			return;
		// SVN error
		default:
			handle_error(String(_("SVN encountered an error")));
			VCS::moveFile(source, dest);
			return;
	}
}

void SubversionVCS::removeFile(const wxFileName& filename)
{
	String name = filename.GetFullPath();
	const Char* name_c[] = {_("svn"), _("rm"), name.c_str(), nullptr};
	handle_warning(String(name_c[0]) + name_c[1] + name_c[2]);
	VCS::removeFile(filename);
	switch (wxExecute(const_cast<Char**>(name_c), wxEXEC_SYNC)) // Once again, yuck
	{
		// Success
		case 0:
			return;
		// Couldn't run SVN
		case -1:
			handle_error(String(_("Can't run SVN.")));
			VCS::removeFile(filename);
			return;
		// SVN error
		default:
			handle_error(String(_("SVN encountered an error")));
			VCS::removeFile(filename);
			return;
	}
}

IMPLEMENT_REFLECTION(SubversionVCS) {
	REFLECT_IF_NOT_READING {
		String type = _("subversion");
		REFLECT(type);
	}
}
// ----------------------------------------------------------------------------- : EOF
