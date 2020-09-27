//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/locale.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <util/io/package_manager.hpp>
#include <util/regex.hpp>
#include <script/to_value.hpp>
#include <wx/wfstream.h>

#include <wx/stdpaths.h>
#if defined(__WXMSW__)
  #include <wx/mstream.h>
#endif

// ----------------------------------------------------------------------------- : Locale class

LocaleP the_locale;

String Locale::typeName() const { return _("locale"); }
Version Locale::fileVersion() const { return file_version_locale; }

LocaleP Locale::byName(const String& name) {
  return package_manager.open<Locale>(name + _(".mse-locale"));
}

IMPLEMENT_REFLECTION_NO_SCRIPT(Locale) {
  REFLECT_BASE(Packaged);
  REFLECT_N("menu",        translations[LOCALE_CAT_MENU]);
  REFLECT_N("help",        translations[LOCALE_CAT_HELP]);
  REFLECT_N("tool",        translations[LOCALE_CAT_TOOL]);
  REFLECT_N("tooltip",     translations[LOCALE_CAT_TOOLTIP]);
  REFLECT_N("label",       translations[LOCALE_CAT_LABEL]);
  REFLECT_N("button",      translations[LOCALE_CAT_BUTTON]);
  REFLECT_N("title",       translations[LOCALE_CAT_TITLE]);
  REFLECT_N("action",      translations[LOCALE_CAT_ACTION]);
  REFLECT_N("error",       translations[LOCALE_CAT_ERROR]);
  REFLECT_N("type",        translations[LOCALE_CAT_TYPE]);
  REFLECT_N("package",     package_translations);
}

IMPLEMENT_REFLECTION_NO_GET_MEMBER(SubLocale) {
  REFLECT_NAMELESS(translations);
}

// ----------------------------------------------------------------------------- : Wildcards

bool match_wildcard(const String& wildcard, const String& name) {
  return Regex(replace_all(replace_all(wildcard, _("."), _("\\.")), _("*"), _(".*"))).matches(name);
}

SubLocaleP find_wildcard(map<String,SubLocaleP>& items, const String& name) {
  FOR_EACH_CONST(i, items) {
    if (i.second && match_wildcard(i.first, name)) return i.second;
  }
  return make_intrusive<SubLocale>(); // so we don't search again
}
SubLocaleP find_wildcard_and_set(map<String,SubLocaleP>& items, const String& name) {
  return items[name] = find_wildcard(items, name);
}

// ----------------------------------------------------------------------------- : Translation

String warn_and_identity(const String& key) {
  queue_message(MESSAGE_WARNING, _("Missing key in locale: ") + key);
  return key;
}
String identity(const String& key) {
  return key;
}

String SubLocale::tr(const String& key, DefaultLocaleFun def) const {
  auto it = translations.find(canonical_name_form(key));
  if (it == translations.end()) {
    return def(key);
  } else {
    return it->second;
  }
}
String SubLocale::tr(const String& subcat, const String& key, DefaultLocaleFun def) const {
  auto it = translations.find(subcat + _("_") + canonical_name_form(key));
  if (it == translations.end()) {
    return def(key);
  } else {
    return it->second;
  }
}

// from util/locale.hpp

String tr(LocaleCategory cat, const String& key, DefaultLocaleFun def) {
  if (!the_locale) return def(key); // no locale loaded (yet)
  return the_locale->translations[cat].tr(key,def);
}

String tr(const Package& pkg, const String& key, DefaultLocaleFun def) {
  if (!the_locale) return def(key);
  SubLocaleP loc = the_locale->package_translations[pkg.relativeFilename()];
  if (!loc) {
    loc = find_wildcard_and_set(the_locale->package_translations, pkg.relativeFilename());
  }
  return loc->tr(key, def);
}

String tr(const Package& pkg, const String& subcat, const String& key, DefaultLocaleFun def) {
  if (!the_locale) return def(key);
  SubLocaleP loc = the_locale->package_translations[pkg.relativeFilename()];
  if (!loc) {
    loc = find_wildcard_and_set(the_locale->package_translations, pkg.relativeFilename());
  }
  return loc->tr(subcat, key, def);
}
