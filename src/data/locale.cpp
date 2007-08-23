//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/locale.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <util/io/package_manager.hpp>
#include <script/to_value.hpp>
#include <wx/wfstream.h>

#include <wx/stdpaths.h>
#if defined(__WXMSW__)
	#include <wx/mstream.h>
#endif

// ----------------------------------------------------------------------------- : Locale class

LocaleP the_locale;

String Locale::typeName() const { return _("locale"); }

LocaleP Locale::byName(const String& name) {
	return packages.open<Locale>(name + _(".mse-locale"));
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
	REFLECT_N("game",        game_translations);
	REFLECT_N("stylesheet",  stylesheet_translations);
	REFLECT_N("symbol font", symbol_font_translations);
}

IMPLEMENT_REFLECTION_NO_GET_MEMBER(SubLocale) {
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
	SubLocaleP loc = the_locale->game_translations[g.name()];
	if (!loc)        return key; // no information on this game
	return loc->tr(key);
}
String tr(const StyleSheet& s, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	SubLocaleP loc = the_locale->stylesheet_translations[s.name()];
	if (!loc)        return key; // no information on this stylesheet
	return loc->tr(key);
}
String tr(const SymbolFont& f, const String& key) {
	if (!the_locale) return key; // no locale loaded (yet)
	SubLocaleP loc = the_locale->symbol_font_translations[f.name()];
	if (!loc)        return key; // no information on this symbol font
	return loc->tr(key);
}


String tr(const Game& g, const String& key, const String& def) {
	if (!the_locale) return def; // no locale loaded (yet)
	SubLocaleP loc = the_locale->game_translations[g.name()];
	if (!loc)        return def; // no information on this game
	return loc->tr(key, def);
}
String tr(const StyleSheet& s, const String& key, const String& def) {
	if (!the_locale) return def; // no locale loaded (yet)
	SubLocaleP loc = the_locale->stylesheet_translations[s.name()];
	if (!loc)        return def; // no information on this stylesheet
	return loc->tr(key, def);
}
String tr(const SymbolFont& f, const String& key, const String& def) {
	if (!the_locale) return def; // no locale loaded (yet)
	SubLocaleP loc = the_locale->symbol_font_translations[f.name()];
	if (!loc)        return def; // no information on this symbol font
	return loc->tr(key, def);
}

// ----------------------------------------------------------------------------- : Validation

DECLARE_POINTER_TYPE(SubLocaleValidator);

class KeyValidator {
  public:
	int  args;
	bool optional;
	DECLARE_REFLECTION();
};

class SubLocaleValidator : public IntrusivePtrBase<SubLocaleValidator> {
  public:
	map<String,KeyValidator> keys; ///< Arg count for each key
	DECLARE_REFLECTION();
};

/// Validation information for locales
class LocaleValidator {
  public:
	map<String, SubLocaleValidatorP> sublocales;
	DECLARE_REFLECTION();
};

template <> void Reader::handle(KeyValidator& k) {
	String v = getValue();
	if (starts_with(v, _("optional, "))) {
		k.optional = true;
		v = v.substr(10);
	} else {
		k.optional = false;
	}
	long l = 0;
	v.ToLong(&l);
	k.args = l;
}
template <> void Writer::handle(const KeyValidator& v) {
	assert(false);
}
IMPLEMENT_REFLECTION_NO_SCRIPT(SubLocaleValidator) {
	REFLECT_NAMELESS(keys);
}
IMPLEMENT_REFLECTION_NO_SCRIPT(LocaleValidator) {
	REFLECT_NAMELESS(sublocales);
}

/// Count "%s" in str
int string_format_args(const String& str) {
	int count = 0;
	bool in_percent = false;
	FOR_EACH_CONST(c, str) {
		if (in_percent) {
			if (c == _('s')) {
				count++;
			}
			in_percent = false;
		} else if (c == _('%')) {
			in_percent = true;
		}
	}
	return count;
}

/// Load a text file from a resource
/** TODO: Move me
 */
InputStreamP load_resource_text(const String& name);
InputStreamP load_resource_text(const String& name) {
	#if defined(__WXMSW__)
		HRSRC hResource = ::FindResource(wxGetInstance(), name, _("TEXT"));
		if ( hResource == 0 ) throw InternalError(String::Format(_("Resource not found: %s"), name));
		HGLOBAL hData = ::LoadResource(wxGetInstance(), hResource);
		if ( hData == 0 ) throw InternalError(String::Format(_("Resource not text: %s"), name));
		char* data = (char *)::LockResource(hData);
		if ( !data ) throw InternalError(String::Format(_("Resource cannot be locked: %s"), name));
		int len = ::SizeofResource(wxGetInstance(), hResource);
		return new_shared2<wxMemoryInputStream>(data, len);
	#else
		static String path = wxStandardPaths::Get().GetDataDir() + _("/resource/") + name;
		return new_shared1<wxFileInputStream>(path);
	#endif
}


DECLARE_TYPEOF(map<String COMMA String>);
DECLARE_TYPEOF(map<String COMMA KeyValidator>);

void Locale::validate(Version ver) {
	// load locale validator
	LocaleValidator v;
	Reader r(load_resource_text(_("expected_locale_keys")), _("expected_locale_keys"));
	r.handle_greedy(v);
	// validate
	String errors;
	// For efficiency, this needs to be parallel to LocaleCategory's values.
	String sublocales[10] = {
		_("menu"),
		_("help"),
		_("tool"),
		_("tooltip"),
		_("label"),
		_("button"),
		_("title"),
		_("type"),
		_("action"),
		_("error")
	};

	for (String * current = sublocales; current < sublocales + 10; ++current) {
		if (v.sublocales[*current])
			errors += translations[current - sublocales].validate(*current,    *v.sublocales[*current]);
		else
			errors += _("\nError validating local file: expected keys file missing \"") + *current + _("\" section.");
	}
	// errors?
	if (!errors.empty()) {
		if (ver != app_version) {
			errors = _("Errors in locale file ") + short_name + _(":") + errors;
		} else {
			errors = _("Errors in locale file ") + short_name +
			         _("\nThis is probably because the locale was made for a different version of MSE.") + errors;
		}
		handle_warning(errors);
	}
}

String SubLocale::validate(const String& name, const SubLocaleValidator& v) const {
	String errors;
	// 1. keys in v but not in this, check arg count
	FOR_EACH_CONST(kc, v.keys) {
		map<String,String>::const_iterator it = translations.find(kc.first);
		if (it == translations.end()) {
			if (!kc.second.optional) {
				errors += _("\n   Missing key:\t\t\t") + name + _(": ") + kc.first;
			}
		} else if (string_format_args(it->second) != kc.second.args) {
			errors += _("\n   Incorrect number of arguments for:\t") + name + _(": ") + kc.first
			       +  String::Format(_("\t  expected: %d, found %d"), kc.second.args, string_format_args(it->second));
		}
	}
	// 2. keys in this but not in v
	FOR_EACH_CONST(kv, translations) {
		map<String,KeyValidator>::const_iterator it = v.keys.find(kv.first);
		if (it == v.keys.end() && !kv.second.empty()) {
			// allow extra keys with empty values as a kind of documentation
			// for example in the help stirngs:
			//   help:
			//       file:
			//       new set: blah blah
			errors += _("\n   Unexpected key:\t\t\t") + name + _(": ") + kv.first;
		}
	}
	return errors;
}
