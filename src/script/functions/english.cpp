//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/tagged_string.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Util

bool is_vowel(Char c) {
	return c == _('a') || c == _('e') || c == _('i') || c == _('o') || c == _('u')
	    || c == _('A') || c == _('E') || c == _('I') || c == _('O') || c == _('U');
}
inline bool is_constant(Char c) {
	return !is_vowel(c);
}

// ----------------------------------------------------------------------------- : Numbers

/// Write a number using words, for example 23 -> "twenty-three"
String english_number(int i) {
	switch (i) {
		case 0:  return _("zero");
		case 1:  return _("one");
		case 2:  return _("two");
		case 3:  return _("three");
		case 4:  return _("four");
		case 5:  return _("five");
		case 6:  return _("six");
		case 7:  return _("seven");
		case 8:  return _("eight");
		case 9:  return _("nine");
		case 10: return _("ten");
		case 11: return _("eleven");
		case 12: return _("twelve");
		case 13: return _("thirteen");
		case 15: return _("fifteen");
		case 18: return _("eighteen");
		case 20: return _("twenty");
		case 30: return _("thirty");
		case 40: return _("forty");
		case 50: return _("fifty");
		case 80: return _("eighty");
		default: {
			if (i < 0 || i >= 100) {
				// number too large, keep as digits
				return (String() << i);
			} else if (i < 20) {
				return english_number(i%10) + _("teen");
			} else if (i % 10 == 0) {
				return english_number(i/10) + _("ty");
			} else {
				// <a>ty-<b>
				return english_number(i/10*10) + _("-") + english_number(i%10);
			}
		}
	}
}

/// Write an ordinal number using words, for example 23 -> "twenty-third"
String english_ordinal(int i) {
	switch (i) {
		case 1:  return _("first");
		case 2:  return _("second");
		case 3:  return _("third");
		case 5:  return _("fifth");
		case 8:  return _("eighth");
		case 9:  return _("ninth");
		case 11: return _("eleventh");
		case 12: return _("twelfth");
		default: {
			if (i <= 0 || i >= 100) {
				// number too large, keep as digits
				return String::Format(_("%dth"), i);
			} else if (i < 20) {
				return english_number(i) + _("th");
			} else if (i % 10 == 0) {
				String num = english_number(i);
				return num.substr(0,num.size()-1) + _("ieth"); // twentieth
			} else {
				// <a>ty-<b>th
				return english_number(i/10*10) + _("-") + english_ordinal(i%10);
			}
		}
	}
}

/// Write a number using words, use "a" for 1
String english_number_a(int i) {
	if (i == 1) return _("a");
	else        return english_number(i);
}
/// Write a number using words, use "" for 1
String english_number_multiple(int i) {
	if (i == 1) return _("");
	else        return english_number(i);
}


// script_english_number_*
String do_english_num(String input, String(*fun)(int)) {
	if (is_substr(input, 0, _("<param-"))) {
		// a keyword parameter, of the form "<param->123</param->"
		size_t start = skip_tag(input, 0);
		if (start != String::npos) {
			size_t end = input.find_first_of(_('<'), start);
			if (end != String::npos) {
				String is = input.substr(start, end - start);
				long i = 0;
				if (is.ToLong(&i)) {
					if (i == 1) {
						return _("<hint-1>") + substr_replace(input, start, end, fun(i));
					} else {
						return _("<hint-2>") + substr_replace(input, start, end, fun(i));
					}
				}
			}
		}
		return _("<hint-2>") + input;
	} else {
		long i = 0;
		if (input.ToLong(&i)) {
			return fun(i);
		}
		return input;
	}
}

SCRIPT_FUNCTION(english_number) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_number));
}
SCRIPT_FUNCTION(english_number_a) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_number_a));
}
SCRIPT_FUNCTION(english_number_multiple) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_number_multiple));
}
SCRIPT_FUNCTION(english_number_ordinal) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_ordinal));
}

// ----------------------------------------------------------------------------- : Singular/plural

String english_singular(const String& str) {
	if (str.size() > 3 && is_substr(str, str.size()-3, _("ies"))) {
		return str.substr(0, str.size() - 3) + _("y");
	} else if (str.size() > 3 && is_substr(str, str.size()-3, _("oes"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 4 && is_substr(str, str.size()-4, _("ches"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 4 && is_substr(str, str.size()-4, _("shes"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 4 && is_substr(str, str.size()-4, _("sses"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 5 && is_substr(str, str.size()-3, _("ves")) && (is_substr(str, str.size()-5, _("el")) || is_substr(str, str.size()-5, _("ar")) )) {
		return str.substr(0, str.size() - 3) + _("f");
	} else if (str.size() > 1 && str.GetChar(str.size() - 1) == _('s')) {
		return str.substr(0, str.size() - 1);
	} else if (str.size() >= 3 && is_substr(str, str.size()-3, _("men"))) {
		return str.substr(0, str.size() - 2) + _("an");
	} else {
		return str;
	}
}
String english_plural(const String& str) {
	if (str.size() > 2) {
		Char a = str.GetChar(str.size() - 2);
		Char b = str.GetChar(str.size() - 1);
		if (b == _('y') && is_constant(a)) {
			return str.substr(0, str.size() - 1) + _("ies");
		} else if (b == _('o') && is_constant(a)) {
			return str + _("es");
		} else if ((a == _('s') || a == _('c')) && b == _('h')) {
			return str + _("es");
		} else if (b == _('s')) {
			return str + _("es");
		} else {
			return str + _("s");
		}
	}
	return str + _("s");
}

// script_english_singular/plural/singplur
String do_english(String input, String(*fun)(const String&)) {
	if (is_substr(input, 0, _("<param-"))) {
		// a keyword parameter, of the form "<param->123</param->"
		size_t start = skip_tag(input, 0);
		if (start != String::npos) {
			size_t end = input.find_first_of(_('<'), start);
			if (end != String::npos) {
				String is = input.substr(start, end - start);
				return substr_replace(input, start, end, fun(is));
			}
		}
		return input; // failed
	} else {
		return fun(input);
	}
}

SCRIPT_FUNCTION(english_singular) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english(input, english_singular));
}
SCRIPT_FUNCTION(english_plural) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english(input, english_plural));
}

// ----------------------------------------------------------------------------- : Hints

/// Process english hints in the input string
/** A hint is formed by
 *    1. an insertion of a parameter, <param-..>...</param->.
 *    2. a <hint-1> or <hint-2> tag
 *   
 *  Hints have the following meaning:
 *   -  "<hint-1>xxx(yyy)zzz"       ---> "xxxzzz"     (singular)
 *   -  "<hint-2>xxx(yyy)zzz"       ---> "xxxyyyzzz"  (plural)
 *   -  "[^., ]a <param-..>[aeiou]" ---> "\1 an \2"   (articla 'an', case insensitive)
 *   -  "<hint-?>"                  ---> ""           (remove <hint>s afterwards)
 *
 *  Note: there is no close tags for hints
 */
String process_english_hints(const String& str) {
	String ret; ret.reserve(str.size());
	// have we seen a <hint-1/2>?
	// 1 for singular, 2 for plural
	int singplur = 0;
	for (size_t i = 0 ; i < str.size() ; ) {
		Char c = str.GetChar(i);
		if (is_substr(str, i, _("<hint-"))) {
			Char h = str.GetChar(i + 6); // hint code
			if (h == _('1')) {
				singplur = 1;
			} else if (h == _('2')) {
				singplur = 2;
			}
			i = skip_tag(str, i);
		} else if (is_substr(str, i, _("<param-"))) {
			size_t after = skip_tag(str, i);
			if (after != String::npos) {
				Char c = str.GetChar(after);
				if (is_vowel(c) && ret.size() >= 2) {
					// a -> an?
					// is there "a" before this?
					String last = ret.substr(ret.size() - 2);
					if ( (ret.size() == 2 || !isAlpha(ret.GetChar(ret.size() - 3))) &&
						(last == _("a ") || last == _("A ")) ) {
						ret.insert(ret.size() - 1, _('n'));
					}
				} else if (is_substr(str, after, _("</param-")) && ret.size() >= 1 &&
				           ret.GetChar(ret.size() - 1) == _(' ')) {
					// empty param, drop space before it
					ret.resize(ret.size() - 1);
				}
			}
			ret += c;
			++i;
		} else if (is_substr(str, i, _("<singular>"))) {
			// singular -> keep, plural -> drop
			size_t start = skip_tag(str, i);
			size_t end   = match_close_tag(str, start);
			if (singplur == 1 && end != String::npos) {
				ret += str.substr(start, end - start);
			}
			singplur = 0;
			i = skip_tag(str, end);
		} else if (is_substr(str, i, _("<plural>"))) {
			// singular -> drop, plural -> keep
			size_t start = skip_tag(str, i);
			size_t end   = match_close_tag(str, start);
			if (singplur == 2 && end != String::npos) {
				ret += str.substr(start, end - start);
			}
			singplur = 0;
			i = skip_tag(str, end);
		} else if (c == _('(') && singplur) {
			// singular -> drop (...), plural -> keep it
			size_t end = str.find_first_of(_(')'), i);
			if (end != String::npos) {
				if (singplur == 2) {
					ret += str.substr(i + 1, end - i - 1);
				}
				i = end + 1;
			} else { // handle like normal
				ret += c;
				++i;
			}
			singplur = 0;
		} else {
			ret += c;
			++i;
		}
	}
	return ret;
}

SCRIPT_FUNCTION(process_english_hints) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(process_english_hints(input));
}

// ----------------------------------------------------------------------------- : Init

void init_script_english_functions(Context& ctx) {
	ctx.setVariable(_("english number"),          script_english_number);
	ctx.setVariable(_("english number a"),        script_english_number_a);
	ctx.setVariable(_("english number multiple"), script_english_number_multiple);
	ctx.setVariable(_("english number ordinal"),  script_english_number_ordinal);
	ctx.setVariable(_("english singular"),        script_english_singular);
	ctx.setVariable(_("english plural"),          script_english_plural);
	ctx.setVariable(_("process english hints"),   script_process_english_hints);
}
