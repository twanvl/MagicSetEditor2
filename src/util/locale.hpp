//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
,	LOCALE_CAT_LABEL
,	LOCALE_CAT_BUTTON
,	LOCALE_CAT_TITLE
,	LOCALE_CAT_TYPE
,	LOCALE_CAT_ACTION
,	LOCALE_CAT_ERROR
,	LOCALE_CAT_MAX
};

/// Translate 'key' in the category 'cat' using the current locale
String tr(LocaleCategory cat, const String& key);

/// Translate 'key' in the for a Game using the current locale
String tr(const Game&, const String& key);
/// Translate 'key' in the for a StyleSheet using the current locale
String tr(const StyleSheet&, const String& key);
/// Translate 'key' in the for a SymbolFont using the current locale
String tr(const SymbolFont&, const String& key);

/// Translate 'key' in the for a Game using the current locale
/** If the key is not found, use the default value */
String tr(const Game&, const String& key, const String& def);
/// Translate 'key' in the for a StyleSheet using the current locale
/** If the key is not found, use the default value */
String tr(const StyleSheet&, const String& key, const String& def);
/// Translate 'key' in the for a SymbolFont using the current locale
/** If the key is not found, use the default value */
String tr(const SymbolFont&, const String& key, const String& def);


/// A localized string for menus/toolbar buttons
#define _MENU_(s)	tr(LOCALE_CAT_MENU,  _(s))
/// A localized string for help/statusbar text
#define _HELP_(s)	tr(LOCALE_CAT_HELP,  _(s))
/// A localized string for tooltip text for toolbar buttons
#define _TOOL_(s)	tr(LOCALE_CAT_TOOL,  _(s))
/// A localized string for labels
#define _LABEL_(s)	tr(LOCALE_CAT_LABEL, _(s))
/// A localized string for buttons/checkboxes/etc.
#define _BUTTON_(s)	tr(LOCALE_CAT_BUTTON,_(s))
/// A localized string for window titles
#define _TITLE_(s)	tr(LOCALE_CAT_TITLE, _(s))
/// A localized string for type names in scripts
#define _TYPE_(s)	tr(LOCALE_CAT_TYPE,  _(s))
/// A localized string for action names
#define _ACTION_(s)	tr(LOCALE_CAT_ACTION, _(s))
/// A localized string for error messages
#define _ERROR_(s)	tr(LOCALE_CAT_ERROR, _(s))

/// A localized string for menus/toolbar buttons, with 1 argument (printf style)
#define _MENU_1_(s,a)		format_string(tr(LOCALE_CAT_MENU,  _(s)), a)

/// A localized string for tooltip text, with 1 argument (printf style)
#define _TOOL_1_(s,a)		format_string(tr(LOCALE_CAT_TOOL,  _(s)), a)

/// A localized string for error messages, with 1 argument (printf style)
#define _ERROR_1_(s,a)		format_string(tr(LOCALE_CAT_ERROR, _(s)), a)
/// A localized string for error messages, with 2 argument (printf style)
#define _ERROR_2_(s,a,b)	format_string(tr(LOCALE_CAT_ERROR, _(s)), a, b)
/// A localized string for error messages, with 3 argument (printf style)
#define _ERROR_3_(s,a,b,c)	format_string(tr(LOCALE_CAT_ERROR, _(s)), a, b, c)

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

// ----------------------------------------------------------------------------- : EOF
#endif
