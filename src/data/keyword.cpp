//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/keyword.hpp>

// ----------------------------------------------------------------------------- : Reflection

IMPLEMENT_REFLECTION(KeywordParam) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(match);
	REFLECT(in_reminder);
}
IMPLEMENT_REFLECTION(KeywordMode) {
	REFLECT(name);
	REFLECT(description);
}
IMPLEMENT_REFLECTION(KeywordExpansion) {
	REFLECT(before);
	REFLECT(after);
	REFLECT(reminder);
}
IMPLEMENT_REFLECTION(Keyword) {
	REFLECT(keyword);
	REFLECT(expansions);
	REFLECT(rules);
	REFLECT(mode);
}

// ----------------------------------------------------------------------------- : Using keywords
