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
{}

StatsDimension::StatsDimension(const Field& field)
	: automatic(true)
	, name         (field.name)
	, description  (field.description)
	, icon_filename(field.icon_filename)
{
	// init script!
}

IMPLEMENT_REFLECTION(StatsDimension) {
	REFLECT(name);
	REFLECT(description);
	REFLECT_N("icon", icon_filename);
	REFLECT(script);
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
	, type(GRAPH_TYPE_BAR)
	, dimensions(1, dim)
{}

IMPLEMENT_REFLECTION_ENUM(GraphType) {
	VALUE_N("bar", GRAPH_TYPE_BAR);
	VALUE_N("pie", GRAPH_TYPE_PIE);
}

IMPLEMENT_REFLECTION(StatsCategory) {
	REFLECT(name);
	REFLECT(description);
	REFLECT_N("icon", icon_filename);
	REFLECT(type);
	REFLECT(dimensions);
}
