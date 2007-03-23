//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/statistics.hpp>
#include <data/field.hpp>

// ----------------------------------------------------------------------------- : Statistics dimension

StatsDimension::StatsDimension()
	: automatic(false)
	, numeric(false)
{}

StatsDimension::StatsDimension(const Field& field)
	: automatic(true)
	, name         (field.name)
	, description  (field.description)
	, icon_filename(field.icon_filename)
	, numeric(false)
{
	// initialize script, card.{field_name}
	Script& s = script.getScript();
	s.addInstruction(I_GET_VAR,  string_to_variable(_("card")));
	s.addInstruction(I_MEMBER_C, field.name);
	s.addInstruction(I_RET);
}

IMPLEMENT_REFLECTION(StatsDimension) {
	if (!automatic) {
		REFLECT(name);
		REFLECT(description);
		REFLECT_N("icon", icon_filename);
		REFLECT(script);
		REFLECT(numeric);
	}
}

// ----------------------------------------------------------------------------- : Statistics category

StatsCategory::StatsCategory()
	: automatic(false)
	, type(GRAPH_TYPE_BAR)
{}

StatsCategory::StatsCategory(const StatsDimensionP& dim)
	: automatic(true)
	, name         (dim->name)
	, description  (dim->description)
	, icon_filename(dim->icon_filename)
	, dimensions(1, dim)
	, type(GRAPH_TYPE_BAR)
{}

IMPLEMENT_REFLECTION_ENUM(GraphType) {
	VALUE_N("bar",     GRAPH_TYPE_BAR);
	VALUE_N("pie",     GRAPH_TYPE_PIE);
	VALUE_N("scatter", GRAPH_TYPE_SCATTER);
}

IMPLEMENT_REFLECTION(StatsCategory) {
	if (!automatic) {
		REFLECT(name);
		REFLECT(description);
		REFLECT_N("icon", icon_filename);
		REFLECT(type);
		REFLECT(dimensions);
	}
}
