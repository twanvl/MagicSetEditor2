//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/locale.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <util/io/package_manager.hpp>
#include <script/to_value.hpp>

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
	REFLECT_N("stylesheet",  stylesheet_translations);
	REFLECT_N("symbol font", symbol_font_translations);
}

IMPLEMENT_REFLECTION_NAMELESS(SubLocale) {
	REFLECT_NAMELESS(translations);
}

// ----------------------------------------------------------------------------- : Translation

String SubLocale::tr(const String& key) {
	map<String,String>::const_iterator it = translations.find(key);
	if (it == translations.end()) {
		return _("missing:") + key;
	} else {
		return it->second;
	}
}
String SubLocale::tr(const String& key, const String& def) {
	map<String,String>::const_iterator it = translations.find(key);
	if (it == translations.end()) {
		return def;
	} else {
		return it->second;
	}
}

// from util/locale.hpp

String tr(LocaleCategory cat, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->translations[cat].tr(key);
}


String tr(const Game& g, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->game_translations[g.name()]->tr(key);
}
String tr(const StyleSheet& s, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->stylesheet_translations[s.name()]->tr(key);
}
String tr(const SymbolFont& f, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->symbol_font_translations[f.name()]->tr(key);
}


String tr(const Game& g, const String& key, const String& def) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->game_translations[g.name()]->tr(key, def);
}
String tr(const StyleSheet& s, const String& key, const String& def) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->stylesheet_translations[s.name()]->tr(key, def);
}
String tr(const SymbolFont& f, const String& key, const String& def) {
	if (!the_locale) return key; // no locale loaded (yet)
	return the_locale->symbol_font_translations[f.name()]->tr(key, def);
}
