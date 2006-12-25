//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/locale.hpp>
#include <util/io/package_manager.hpp>
#include <script/value.hpp>

// ----------------------------------------------------------------------------- : Locale class

LocaleP the_locale;

String Locale::typeName() const { return _("locale"); }

LocaleP Locale::byName(const String& name) {
	return packages.open<Locale>(name + _(".mse-locale"));
}

IMPLEMENT_REFLECTION(Locale) {
	REFLECT_BASE(Packaged);
	REFLECT_N("menu",   translations[LOCALE_CAT_MENU]);
	REFLECT_N("help",   translations[LOCALE_CAT_HELP]);
	REFLECT_N("tool",   translations[LOCALE_CAT_TOOL]);
	REFLECT_N("label",  translations[LOCALE_CAT_LABEL]);
	REFLECT_N("button", translations[LOCALE_CAT_BUTTON]);
	REFLECT_N("title",  translations[LOCALE_CAT_TITLE]);
	REFLECT_N("action", translations[LOCALE_CAT_ACTION]);
	REFLECT_N("error",  translations[LOCALE_CAT_ERROR]);
	REFLECT_N("type",   translations[LOCALE_CAT_TYPE]);
	REFLECT_N("game",   game_translations);
}

IMPLEMENT_REFLECTION_NAMELESS(GameLocale) {
	REFLECT_NAMELESS(translations);
}

// ----------------------------------------------------------------------------- : Translation

// from util/locale.hpp

String tr(LocaleCategory cat, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	map<String,String>::const_iterator it = the_locale->translations[cat].find(key);
	if (it == the_locale->translations[cat].end()) return _("missing:") + key;
	return it->second;
}
