//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/installer.hpp>
#include <script/to_value.hpp>

// ----------------------------------------------------------------------------- : Installer

String Installer::typeName() const { return _("installer"); }

IMPLEMENT_REFLECTION(Installer) {
	REFLECT_BASE(Packaged);
	REFLECT(packages);
}
