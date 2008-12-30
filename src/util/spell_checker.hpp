//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_SPELL_CHECKER
#define HEADER_UTIL_SPELL_CHECKER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include "hunspell.hxx"

DECLARE_POINTER_TYPE(SpellChecker);

// ----------------------------------------------------------------------------- : Spell checker

/// A spelling checker for a particular language
class SpellChecker : public Hunspell, public IntrusivePtrBase<SpellChecker> {
  public:
	/// Get a SpellChecker object for the given language.
	/** Note: This is not threadsafe yet */
	static SpellChecker& get(const String& language);
	/// Get a SpellChecker object for the given language and filename
	/** Note: This is not threadsafe yet */
	static SpellChecker& get(const String& filename, const String& language);
	/// Destroy all cached SpellChecker objects
	static void destroyAll();
	
	/// Check the spelling of a single word
	bool spell(const String& word);
	/// Check the spelling of a single word, ignore punctuation
	bool spell_with_punctuation(const String& word);
	
  private:
	/// Convert between String and dictionary encoding
	wxCSConv encoding;
	
	SpellChecker(const char* aff_path, const char* dic_path);
	static map<String,SpellCheckerP> spellers; //< Cached checkers for each language
};

// ----------------------------------------------------------------------------- : EOF
#endif
