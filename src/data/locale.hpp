//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_LOCALE
#define HEADER_DATA_LOCALE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/locale.hpp>
#include <util/reflect.hpp>
#include <util/io/package.hpp>

DECLARE_POINTER_TYPE(Locale);
DECLARE_POINTER_TYPE(SubLocale);
DECLARE_POINTER_TYPE(SubLocaleValidator);

// ----------------------------------------------------------------------------- : Locale class

/// Translations of the texts of a game/stylesheet/symbolfont
class SubLocale : public IntrusivePtrBase<SubLocale> {
  public:
	map<String,String> translations;
	
	/// Translate a key, if not found, apply the default function to the key
	String tr(const String& key, DefaultLocaleFun def);
	String tr(const String& subcat, const String& key, DefaultLocaleFun def);
	
	/// Is this a valid sublocale? Returns errors
	String validate(const String& name, const SubLocaleValidatorP&) const;
	
	DECLARE_REFLECTION();
};

/// A collection of translations of messages
class Locale : public Packaged {
  public:
	/// Translations of UI strings in each category
	SubLocale              translations[LOCALE_CAT_MAX];
	/// Translations of Game specific texts, by game name
	map<String,SubLocaleP> game_translations;
	/// Translations of StyleSheet specific texts, by stylesheet name
	map<String,SubLocaleP> stylesheet_translations;
	/// Translations of SymbolFont specific texts, by symbol font name
	map<String,SubLocaleP> symbol_font_translations;
	
	/// Open a locale with the given name
	static LocaleP byName(const String& name);
	
	/// Validate that the locale is valid for this MSE version
	virtual void validate(Version = app_version);
	
  protected:
	String typeName() const;
	DECLARE_REFLECTION();
};

/// The global locale object
extern LocaleP the_locale;

// ----------------------------------------------------------------------------- : EOF
#endif
