//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/spell_checker.hpp>
#include <util/string.hpp>
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

void SpellChecker::destroyAll() {
	spellers.clear();
}

// ----------------------------------------------------------------------------- : Spell checker : use

bool SpellChecker::spell(const String& word) {
	if (word.empty()) return true; // empty word is okay
	// fix curly quotes, especially apstrophes
	String fixed;
	FOR_EACH_CONST(c,word) {
		if (c == LEFT_SINGLE_QUOTE || c == RIGHT_SINGLE_QUOTE) {
			fixed += _('\'');
		} else if (c == LEFT_DOUBLE_QUOTE || c == RIGHT_DOUBLE_QUOTE) {
			fixed += _('\"');
		} else if (c == 0x00C6) {
			// expand ligatures, TODO: put this in a better place
			fixed += _("Ae");
		} else if (c == 0x0132) {
			fixed += _("IJ");
		} else if (c == 0x0152) {
			fixed += _("Oe");
		} else if (c == 0xFB01) {
			fixed += _("fi");
		} else if (c == 0xFB02) {
			fixed += _("fl");
		} else {
			fixed += c;
		}
	}
	// convert encoding
	#ifdef UNICODE
		wxCharBuffer str = fixed.mb_str(encoding);
	#else
		wxCharBuffer str = fixed.mb_str(encoding);
	#endif
	if (*str == '\0') {
		// If encoding fails we get an empty string, since the word was not empty this can never happen
		// words that can't be encoded are not in the dictionary, so they are wrong.
		return false;
	}
	return Hunspell::spell(str);
}

bool SpellChecker::spell_with_punctuation(const String& word) {
	size_t start = 0, end = String::npos;
	trim_punctuation(word, start, end);
	if (start >= end) return true; // just punctuation is wrong
	return spell(word.substr(start,end-start));
}
