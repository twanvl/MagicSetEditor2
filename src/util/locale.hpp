//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_LOCALE
#define HEADER_UTIL_LOCALE

/** @file util/locale.hpp
 *
 *  @brief Utilities for localisation of text.
 *  Whenever text is used that can be translated to another language (and is not code related)
 *  one of the macros from this file should be used
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/string.hpp>

class Game;
class StyleSheet;
class SymbolFont;

// ----------------------------------------------------------------------------- : Localisation macros

enum LocaleCategory
{	LOCALE_CAT_MENU
,	LOCALE_CAT_HELP
,	LOCALE_CAT_TOOL
,	LOCALE_CAT_TOOLTIP
,	LOCALE_CAT_LABEL
,	LOCALE_CAT_BUTTON
,	LOCALE_CAT_TITLE
,	LOCALE_CAT_TYPE
,	LOCALE_CAT_ACTION
,	LOCALE_CAT_ERROR
,	LOCALE_CAT_MAX
};

typedef String (*DefaultLocaleFun)(const String&);
/// Return the input and issue a warning
String warn_and_identity(const String&);

/// Translate 'key' in the category 'cat' using the current locale
String tr(LocaleCategory cat, const String& key, DefaultLocaleFun def = warn_and_identity);

/// Translate 'key' in the for a Game using the current locale
String tr(const Game&, const String& key, DefaultLocaleFun def);
/// Translate 'key' in the for a StyleSheet using the current locale
String tr(const StyleSheet&, const String& key, DefaultLocaleFun def);
/// Translate 'key' in the for a SymbolFont using the current locale
String tr(const SymbolFont&, const String& key, DefaultLocaleFun def);

/// Translate 'key' in the for a Game using the current locale
String tr(const Game&, const String& subcat, const String& key, DefaultLocaleFun def);
/// Translate 'key' in the for a StyleSheet using the current locale
String tr(const StyleSheet&, const String& subcat, const String& key, DefaultLocaleFun def);
/// Translate 'key' in the for a SymbolFont using the current locale
String tr(const SymbolFont&, const String& subcat, const String& key, DefaultLocaleFun def);

/// A localized string for menus
#define _MENU_(s)    tr(LOCALE_CAT_MENU,      _(s))
/// A localized string for help/statusbar text
#define _HELP_(s)    tr(LOCALE_CAT_HELP,      _(s))
/// A localized string for the text of toolbar buttons
#define _TOOL_(s)    tr(LOCALE_CAT_TOOL,      _(s))
/// A localized string for tooltip text for toolbar buttons
#define _TOOLTIP_(s) tr(LOCALE_CAT_TOOLTIP,   _(s))
/// A localized string for labels
#define _LABEL_(s)   tr(LOCALE_CAT_LABEL,     _(s))
/// A localized string for buttons/checkboxes/etc.
#define _BUTTON_(s)  tr(LOCALE_CAT_BUTTON,    _(s))
/// A localized string for window titles
#define _TITLE_(s)   tr(LOCALE_CAT_TITLE,     _(s))
/// A localized string for type names in scripts
#define _TYPE_(s)    tr(LOCALE_CAT_TYPE,      _(s))
/// A localized string for action names
#define _ACTION_(s)  tr(LOCALE_CAT_ACTION,    _(s))
/// A localized string for error messages
#define _ERROR_(s)   tr(LOCALE_CAT_ERROR,     _(s))

/// A localized string for menus, with 1 argument (printf style)
#define _MENU_1_(s,a)		format_string(_MENU_(s),    a)
/// A localized string for context menus, contains no "\tshortcut"
#define _CONTEXT_MENU_(s)   remove_shortcut(_MENU_(s))

/// A localized string for tooltip text, with 1 argument (printf style)
#define _HELP_1_(s,a)		format_string(_HELP_(s),    a)

/// A localized string for tooltip text, with 1 argument (printf style)
#define _TOOLTIP_1_(s,a)	format_string(_TOOLTIP_(s), a)

/// A localized string for tooltip labels, with 1 argument (printf style)
#define _LABEL_1_(s,a)		format_string(_LABEL_(s),   a)

/// A localized string for button text, with 1 argument (printf style)
#define _BUTTON_1_(s,a)		format_string(_BUTTON_(s), a)

/// A localized string for window titles, with 1 argument (printf style)
#define _TITLE_1_(s,a)		format_string(_TITLE_(s), a)

/// A localized string for type names in scripts, with 1 argument (printf style)
#define _TYPE_1_(s,a)		format_string(_TYPE_(s), a)

/// A localized string for action names, with 1 argument (printf style)
#define _ACTION_1_(s,a)		format_string(_ACTION_(s), a)

/// A localized string for error messages, with 1 argument (printf style)
#define _ERROR_1_(s,a)		format_string(_ERROR_(s),   a)
/// A localized string for error messages, with 2 argument (printf style)
#define _ERROR_2_(s,a,b)	format_string(_ERROR_(s),   a, b)
/// A localized string for error messages, with 3 argument (printf style)
#define _ERROR_3_(s,a,b,c)	format_string(_ERROR_(s),   a, b, c)
/// A localized string for error messages, with 4 argument (printf style)
#define _ERROR_4_(s,a,b,c,d) format_string(_ERROR_(s),   a, b, c, d)

/// Format a string
/** Equivalent to sprintf / String::Format, but allows strings to be passed as arguments (gcc)
 */
inline String format_string(const String& format, ...) {
	va_list args;
	va_start(args, format);
	String res = String::Format(format, args);
	va_end(args);
	return res;
}
inline String format_string(const String& format, const String& a0) {
	return String::Format(format, a0.c_str());
}
inline String format_string(const String& format, const String& a0, const String& a1) {
	return String::Format(format, a0.c_str(), a1.c_str());
}
inline String format_string(const String& format, const String& a0, const String& a1, const String& a2) {
	return String::Format(format, a0.c_str(), a1.c_str(), a2.c_str());
}
inline String format_string(const String& format, const String& a0, const String& a1, const String& a2, const String& a3) {
	return String::Format(format, a0.c_str(), a1.c_str(), a2.c_str(), a3.c_str());
}

// ----------------------------------------------------------------------------- : EOF
#endif
