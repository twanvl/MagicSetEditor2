//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs.hpp>
#include <util/vcs/subversion.hpp>
#include <util/vcs/git.hpp>

// ----------------------------------------------------------------------------- : Reflection

template <>
VCSP read_new<VCS>(Reader& reader) {
  // there must be a type specified
  String type;
  reader.handle(_("type"), type);
  if      (type == _("none")) return make_intrusive<VCS>();
  else if (type == _("subversion")) return make_intrusive<SubversionVCS>();
  else if (type == _("git")) return make_intrusive<GitVCS>();
  else if (type.empty()) {
    reader.warning(_ERROR_1_("expected key", _("version control system")));
    throw ParseError(_ERROR_("aborting parsing"));
  } else {
    reader.warning(format_string(_("Unsupported version control type: '%s'"), type));
    throw ParseError(_ERROR_("aborting parsing"));
  }
}

IMPLEMENT_REFLECTION(VCS) {
  REFLECT_IF_NOT_READING {
    String type = _("none");
    REFLECT(type);
  }
}

template <>
void Reader::handle(VCSP& pointer) {
  pointer = read_new<VCS>(*this);
  handle(*pointer);
}

// ----------------------------------------------------------------------------- : EOF
