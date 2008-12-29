//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/spell_checker.hpp>
#include <util/io/package_manager.hpp>

// ----------------------------------------------------------------------------- : Spell checker : construction

map<String,SpellCheckerP> SpellChecker::spellers;

SpellChecker& SpellChecker::get(const String& language) {
	SpellCheckerP& speller = spellers[language];
	if (!speller) {
		String local_dir  = package_manager.getDictionaryDir(true);
		String global_dir = package_manager.getDictionaryDir(false);
		String aff_path = language + _(".aff");
		String dic_path = language + _(".dic");
		if (wxFileExists(local_dir + aff_path) && wxFileExists(local_dir + dic_path)) {
			speller = SpellCheckerP(new SpellChecker((local_dir + aff_path).mb_str(),
			                                         (local_dir + dic_path).mb_str()));
		} else if (wxFileExists(global_dir + aff_path) && wxFileExists(global_dir + dic_path)) {
			speller = SpellCheckerP(new SpellChecker((global_dir + aff_path).mb_str(),
			                                         (global_dir + dic_path).mb_str()));
		} else {
			throw Error(_("Dictionary not found for language: ") + language);
		}
	}
	return *speller;
}

SpellChecker::SpellChecker(const char* aff_path, const char* dic_path)
	: Hunspell(aff_path,dic_path)
	, encoding(String(get_dic_encoding(), IF_UNICODE(wxConvLibc, wxSTRING_MAXLEN)))
{}

void SpellChecker::destroy() {
	spellers.clear();
}

// ----------------------------------------------------------------------------- : Spell checker : use

bool SpellChecker::spell(const String& word) {
	return Hunspell::spell(word.mb_str(encoding));
}

const String word_start = String(_("[({\"\'"))     + LEFT_SINGLE_QUOTE  + LEFT_DOUBLE_QUOTE;
const String word_end   = String(_("])}.,;:\"\'")) + RIGHT_SINGLE_QUOTE + RIGHT_DOUBLE_QUOTE;

bool SpellChecker::spell_with_punctuation(const String& word) {
	size_t first = word.find_first_not_of(word_start);
	size_t last  = word.find_last_not_of(word_end);
	if (first > last) return false; // just punctuation is incorrect
	return spell(word.substr(first, last-first+1));
}
