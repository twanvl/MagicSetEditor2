//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs.hpp>
#include <util/vcs/subversion.hpp>

// ----------------------------------------------------------------------------- : Reflection

template <>
VCSP read_new<VCS>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
	if      (type == _("none"))				return intrusive(new VCS);
	else if (type == _("subversion"))		return intrusive(new SubversionVCS);
	else if (type.empty()) {
		reader.warning(_ERROR_1_("expected key", _("version control system")));
		throw ParseError(_ERROR_("aborting parsing"));
	} else {
		reader.warning(_ERROR_1_("unsupported version control type", type));
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
