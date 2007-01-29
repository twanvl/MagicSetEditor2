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
	REFLECT(script);
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

// backwards compatability
template <typename T> void read_compat(T&, const Keyword*) {}
void read_compat(Reader& tag, Keyword* k) {
	String separator, parameter, reminder;
	REFLECT(separator);
	REFLECT(parameter);
	REFLECT(reminder);
	if (!separator.empty() || !parameter.empty() || !reminder.empty()) {
		// old style keyword declaration, no separate expansion
		KeywordExpansionP e(new KeywordExpansion);
		size_t start = separator.find_first_of('[');
		size_t end   = separator.find_first_of(']');
		if (start != String::npos && end != String::npos) {
			e->after += separator.substr(start + 1, end - start - 1);
		}
		if (!parameter.empty()) {
			e->after += _("<param>") + parameter + _("</param>");
		}
		e->reminder.set(reminder);
	}
}

IMPLEMENT_REFLECTION(Keyword) {
	REFLECT(keyword);
	read_compat(tag, this);
	REFLECT(expansions);
	REFLECT(rules);
	REFLECT(mode);
}

// ----------------------------------------------------------------------------- : Using keywords
