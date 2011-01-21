//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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

DECLARE_TYPEOF(map<String COMMA SubLocaleP>);

// ----------------------------------------------------------------------------- : Locale class

// when reading, ignore "#_ADD" start of line pragmas

typedef void (*ReaderPragmaHandler)(String&);
DECLARE_DYNAMIC_ARG(ReaderPragmaHandler,reader_pragma_handler);

void ignore_add_pragma(String& str) {
	if      (starts_with(str,_("#_ADD "))) str = str.substr(6);
	else if (starts_with(str,_("#_ADD")))  str = str.substr(5);
	else if (starts_with(str,_("#_DEL")))  str.clear();
}

// ----------------------------------------------------------------------------- : Locale class

LocaleP the_locale;

String Locale::typeName() const { return _("locale"); }
Version Locale::fileVersion() const { return file_version_locale; }

LocaleP Locale::byName(const String& name) {
	return package_manager.open<Locale>(name + _(".mse-locale"));
}

IMPLEMENT_REFLECTION_NO_SCRIPT(Locale) {
	REFLECT_BASE(Packaged);
	WITH_DYNAMIC_ARG(reader_pragma_handler, ignore_add_pragma);
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
	return intrusive(new SubLocale()); // so we don't search again
}
SubLocaleP find_wildcard_and_set(map<String,SubLocaleP>& items, const String& name) {
	return items[name] = find_wildcard(items, name);
}

// ----------------------------------------------------------------------------- : Translation

String warn_and_identity(const String& key) {
	queue_message(MESSAGE_WARNING, _("Missing key in locale: ") + key);
	return key;
}

String SubLocale::tr(const String& key, DefaultLocaleFun def) {
	map<String,String>::const_iterator it = translations.find(key);
	if (it == translations.end()) {
		return def(key);
	} else {
		return it->second;
	}
}
String SubLocale::tr(const String& subcat, const String& key, DefaultLocaleFun def) {
	map<String,String>::const_iterator it = translations.find(subcat + _(" ") + key);
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
		return shared(new wxMemoryInputStream(data, len));
	#else
        static String path = wxStandardPaths::Get().GetDataDir() + _("/resource/");
        static String local_path = wxStandardPaths::Get().GetUserDataDir() + _("/resource/");
        if (wxFileExists(path + name)) return shared(new wxFileInputStream(path + name));
        else return shared(new wxFileInputStream(local_path + name));
	#endif
}


DECLARE_TYPEOF(map<String COMMA String>);
DECLARE_TYPEOF(map<String COMMA KeyValidator>);

void Locale::validate(Version ver) {
	Packaged::validate(ver);
	// load locale validator
	LocaleValidator v;
	Reader r(load_resource_text(_("expected_locale_keys")), nullptr, _("expected_locale_keys"));
	r.handle_greedy(v);
	// validate
	String errors;
	errors += translations[LOCALE_CAT_MENU   ].validate(_("menu"),    v.sublocales[_("menu")   ]);
	errors += translations[LOCALE_CAT_HELP   ].validate(_("help"),    v.sublocales[_("help")   ]);
	errors += translations[LOCALE_CAT_TOOL   ].validate(_("tool"),    v.sublocales[_("tool")   ]);
	errors += translations[LOCALE_CAT_TOOLTIP].validate(_("tooltip"), v.sublocales[_("tooltip")]);
	errors += translations[LOCALE_CAT_LABEL  ].validate(_("label"),   v.sublocales[_("label")  ]);
	errors += translations[LOCALE_CAT_BUTTON ].validate(_("button"),  v.sublocales[_("button") ]);
	errors += translations[LOCALE_CAT_TITLE  ].validate(_("title"),   v.sublocales[_("title")  ]);
	errors += translations[LOCALE_CAT_ACTION ].validate(_("action"),  v.sublocales[_("action") ]);
	errors += translations[LOCALE_CAT_ERROR  ].validate(_("error"),   v.sublocales[_("error")  ]);
	errors += translations[LOCALE_CAT_TYPE   ].validate(_("type"),    v.sublocales[_("type")   ]);
	// errors?
	if (!errors.empty()) {
		if (ver != file_version_locale) {
			errors = _("Errors in locale file ") + short_name + _(":") + errors;
		} else {
			errors = _("Errors in locale file ") + short_name +
			         _("\nThis is probably because the locale was made for a different version of MSE.") + errors;
		}
	} else if (ver != file_version_locale) {
			errors = _("Errors in locale file ") + short_name + _(":")
			       + _("\n  Locale file out of date, expected: mse version: ") + file_version_locale.toString()
			       + _("\n  found: ") + ver.toString();
	}
	if (!errors.empty()) {
		queue_message(MESSAGE_WARNING, errors);
	}
}

String SubLocale::validate(const String& name, const SubLocaleValidatorP& v) const {
	if (!v) {
		return _("\nInternal error validating local file: expected keys file missing for \"") + name + _("\" section.");
	}
	String errors;
	// 1. keys in v but not in this, check arg count
	FOR_EACH_CONST(kc, v->keys) {
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
		map<String,KeyValidator>::const_iterator it = v->keys.find(kv.first);
		if (it == v->keys.end() && !kv.second.empty()) {
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
