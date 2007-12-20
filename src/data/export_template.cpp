//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/export_template.hpp>
#include <data/game.hpp>
#include <data/field.hpp>

// ----------------------------------------------------------------------------- : Export template, basics

ExportTemplate::ExportTemplate()
	: create_directory(false)
	, file_type(_("HTML files (*.html)|*.html"))
{}

String ExportTemplate::typeNameStatic() { return _("export-template"); }
String ExportTemplate::typeName() const { return _("export-template"); }

void ExportTemplate::validate(Version) {
	if (!game) {
		throw Error(_ERROR_("no game specified for export template"));
	}
	// an export template depends on the game it is made for
	requireDependency(game.get());
}


IMPLEMENT_REFLECTION(ExportTemplate) {
	REFLECT_BASE(Packaged);
	REFLECT(game);
	REFLECT(file_type);
	REFLECT(create_directory);
	REFLECT(option_fields);
	REFLECT_IF_READING option_style.init(option_fields);
	REFLECT(option_style);
	REFLECT(script);
}

// ----------------------------------------------------------------------------- : ExportInfo

IMPLEMENT_DYNAMIC_ARG(ExportInfo*, export_info, nullptr);
