//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/tagged_string.hpp>
#include <util/error.hpp>

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

SCRIPT_FUNCTION(english_number) {
	SCRIPT_PARAM(int, input);
	SCRIPT_RETURN(english_number(input));
}

SCRIPT_FUNCTION(english_number_a) {
	SCRIPT_PARAM(int, input);
	if (input == 1) {
		SCRIPT_RETURN(_("a"));
	} else {
		SCRIPT_RETURN(english_number(input));
	}
}

// ----------------------------------------------------------------------------- : A/an


// ----------------------------------------------------------------------------- : Singular/plural

SCRIPT_FUNCTION(english_singular) {
	throw InternalError(_("TODO"));
}

SCRIPT_FUNCTION(english_plural) {
	throw InternalError(_("TODO"));
}

// ----------------------------------------------------------------------------- : Hints

// insert a hint, <hint-1> for singular, <hint-2> otherwise
SCRIPT_FUNCTION(plural_hint) {
	SCRIPT_PARAM(int, input);
	SCRIPT_RETURN(input == 1 ? _("<hint-1>") : _("<hint-2>"));
}

/// Process english hints in the input string
/** Hints have the following meaning:
 *   -  "<hint-1>xxx(yyy)zzz"     ---> "xxxzzz"     (singular)
 *   -  "<hint-2>xxx(yyy)zzz"     ---> "xxxyyyzzz"  (plural)
 *   -  "[^., ]a <hint-v>[aeiou]" ---> "\1 an \2"   (articla 'an', case insensitive)
 *   -  "<hint-?>"                ---> ""           (remove <hint>s afterwards)
 *
 *  Note: there is no close tags for hints
 */
String process_english_hints(const String& str) {
	String ret; ret.reserve(str.size());
	int singplur = 0; // 1 for singular, 2 for plural
	for (size_t i = 0 ; i < str.size() ; ) {
		Char c = str.GetChar(i);
		if (i + 6 < str.size() && is_substr(str, i, _("<hint-"))) {
			Char h = str.GetChar(i + 6); // hint code
			if (h == _('1')) {
				singplur = 1;
			} else if (h == _('2')) {
				singplur = 2;
			} else if (h == _('v')) {
				// TODO
			}
			i = skip_tag(str, i);
		} else if (c == _('(') && singplur) {
			// singular -> drop (...), plural -> keep it
			// TODO
		} else {
			ret += c;
			++i;
		}
	}
	return ret; // TODO
}


// ----------------------------------------------------------------------------- : Init

void init_script_english_functions(Context& ctx) {
	ctx.setVariable(_("english number"),   script_english_number);
	ctx.setVariable(_("english number a"), script_english_number_a);
}
