//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs/git.hpp>
#include "wx/process.h"

// ----------------------------------------------------------------------------- : SVN File Manipulation

// Maybe consider linking against libgit2, and making this a core feature?
// food for future thought.  Could be useful for template devs?
bool run_git(const Char** arguments, const wxString wd) {
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
    // Couldn't run Git
    case -1:
        handle_error(String(_("Can't run Git.")));
        delete process;
        delete env;
        return false;
    // Git error
    default:
        String error = String(_("Git encountered an error:\n"));
        wxString log;
        wxInputStream* err = process->GetErrorStream();

        wxTextInputStream tStream(*err);
        while (!err->Eof())
        {
            log = tStream.ReadLine();
            error.append(log).append('\n');
        }
        handle_error(error);
        delete process;
        delete env;
        return false;
    }
}

void GitVCS::addFile(const wxFileName& filename)
{
    String name = filename.GetFullPath();
    wxString wd = filename.GetPath(true);
    const Char* name_c[] = { _("git"), _("add"), name.c_str(), nullptr };
    if (!run_git(name_c, wd)) {
        VCS::addFile(filename);
    }
}

void GitVCS::moveFile(const wxFileName& source, const wxFileName& dest)
{
    String source_name = source.GetFullPath(), dest_name = dest.GetFullPath();
    const Char* name_c[] = { _("git"), _("mv"), source_name.c_str(), dest_name.c_str(), nullptr };
    wxString wd = source.GetPath(true);
    if (!run_git(name_c, wd)) {
        VCS::moveFile(source, dest);
    }
}

void GitVCS::removeFile(const wxFileName& filename)
{
    String name = filename.GetFullPath();
    const Char* name_c[] = { _("git"), _("rm"), name.c_str(), nullptr };
    wxString wd = filename.GetPath(true);
    // `git rm` only removes it from the index.  We still need to removed it from the filesystem.
    VCS::removeFile(filename);
    if (!run_git(name_c, wd)) {
        VCS::removeFile(filename);
    }
}

void GitVCS::updateFile(const wxFileName& filename) {
    // Git needs you to explicitly stage changes
    this->addFile(filename);
}

void GitVCS::commit(const String& directory) {
    const Char* name_c[] = { _("git"), _("commit"), nullptr };
    run_git(name_c, directory);
    const Char* push_c[] = { _("git"), _("push"), nullptr };
    run_git(push_c, directory);
}

void GitVCS::pull(const String& directory) {
    // TODO: Fetch, check status, and only pull if no conflicts.
    const Char* name_c[] = { _("git"), _("pull"), nullptr };
    run_git(name_c, directory);
}


IMPLEMENT_REFLECTION(GitVCS) {
    REFLECT_IF_NOT_READING{
      String type = _("git");
      REFLECT(type);
    }
}

// ----------------------------------------------------------------------------- : EOF
