//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class Locale;

// ----------------------------------------------------------------------------- : LocalizedString

/// Translations of a text in a template
class LocalizedString {
public:
  String default_; //< Value in all other locales
  unordered_map<String, String> translations;

  /// Translate
  String const& get(Locale const& locale) const;
  String const& get(String const& locale) const;
  String const& get() const;

  bool empty() const { return default_.empty(); }
};

#define REFLECT_LOCALIZED_N(name, var) \
  do { \
    handler.handle(name, var.default_); \
    handler.handle(_("localized_") name, var.translations); \
  } while (0)

#define REFLECT_LOCALIZED(var) REFLECT_LOCALIZED_N(_(#var), var)
