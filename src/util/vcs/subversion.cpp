//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs/subversion.hpp>
#include <wx/process.h>

// ----------------------------------------------------------------------------- : SVN File Manipulation

bool run_svn(const Char** arguments, const wxString wd) {
  wxProcess* process = new wxProcess(wxPROCESS_REDIRECT);
  process->Redirect();
  wxExecuteEnv* env = new wxExecuteEnv();
  env->cwd = wd;
  switch (wxExecute(const_cast<Char**>(arguments), wxEXEC_SYNC, process, env)) { // Yuck, const_cast
    // Success
    case 0:
      delete process;
      delete env;
      return true;
    // Couldn't run SVN
    case -1:
      handle_error(String(_("Can't run SVN.")));
      delete process;
      delete env;
      return false;
    // SVN error
    default:
      String error = String(_("SVN encountered an error\n"));
      wxString log;
      wxInputStream* err = process->GetErrorStream();
      if (err != NULL) {
        wxTextInputStream tStream(*err);
        while (!err->Eof())
        {
            log = tStream.ReadLine();
            error.append(log).append('\n');
        }
      }

      handle_error(error);
      delete process;
      delete env;
      return false;
  }
  
}

void SubversionVCS::addFile(const wxFileName& filename)
{
  String name = filename.GetFullPath();
  const Char* name_c[] = {_("svn"), _("add"), name.c_str(), nullptr};
  wxString wd = filename.GetPath(true);
  if (!run_svn(name_c, wd)) {
    VCS::addFile(filename);
  }
}

void SubversionVCS::moveFile(const wxFileName& source, const wxFileName& dest)
{
  String source_name = source.GetFullPath(), dest_name = dest.GetFullPath();
  const Char* name_c[] = {_("svn"), _("mv"), source_name.c_str(), dest_name.c_str(), nullptr};
  wxString wd = source.GetPath(true);
  if (!run_svn(name_c, wd)) {
    VCS::moveFile(source, dest);
  }
}

void SubversionVCS::removeFile(const wxFileName& filename)
{
  String name = filename.GetFullPath();
  const Char* name_c[] = {_("svn"), _("rm"), name.c_str(), nullptr};
  wxString wd = filename.GetPath(true);
  queue_message(MESSAGE_WARNING, String(name_c[0]) + name_c[1] + name_c[2]);
  // TODO: do we really need to remove the file before calling "svn remove"?
  VCS::removeFile(filename);
  if (!run_svn(name_c, wd)) {
    VCS::removeFile(filename);
  }
}

void SubversionVCS::commit(const String& directory) {
    // Implementing this might be a breaking change to existing svn users (if there are any)

    //const Char* name_c[] = { _("svn"), _("commit"), nullptr };
    //run_svn(name_c, directory);
}

void SubversionVCS::pull(const String& directory) {
    const Char* name_c[] = { _("svn"), _("up"), nullptr };
    run_svn(name_c, directory);
}

IMPLEMENT_REFLECTION(SubversionVCS) {
  REFLECT_IF_NOT_READING {
    String type = _("subversion");
    REFLECT(type);
  }
}

// ----------------------------------------------------------------------------- : EOF
