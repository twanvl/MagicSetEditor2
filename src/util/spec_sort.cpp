//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/spec_sort.hpp>
#include <util/error.hpp>

const Char REMOVED     = _('\2');
const Char PLACEHOLDER = _('\3');

String spec_sort(const String& spec, String& input, String& ret);

// ----------------------------------------------------------------------------- : Iterator for reading specs

/// Iterator over a sort specification (for spec_sort)
class SpecIterator {
  public:
	SpecIterator(const String& spec, size_t pos = 0)
		: spec(spec), pos(pos)
	{}
	
	Char value;				///< Current character
	bool escaped;			///< Was the current character escaped?
	bool preceded_by_space;	///< Was there a ' ' before this character?
	
	/// Move to the next item in the specification.
	/** returns false if we are at the end or encounter close.
	 */
	bool nextUntil(Char close, bool skip_space = true) {
		if (pos >= spec.size()) {
			value = 0;
			if (close == 0) {
				return false;
			} else {
				throw ParseError(String::Format(_("Expected '%c' in sort_rule specification"),close));
			}
		}
		value = spec.GetChar(pos++);
		preceded_by_space = false;
		// skip whitespace
		if (skip_space) {
			while (value == _(' ')) {
				if (pos >= spec.size()) {
					if (close == 0) {
						return false;
					} else {
						throw ParseError(String::Format(_("Expected '%c' in sort_rule specification"),close));
					}
				}
				value = spec.GetChar(pos++);
				preceded_by_space = true;
			}
		}
		// escape?
		if (value == _('\\')) {
			escaped = true;
			if (pos >= spec.size()) {
				throw ParseError(String::Format(_("Expected '%c' in sort_rule specification"),close));
			}
			value = spec.GetChar(pos++);
		} else {
			escaped = false;
			if (value == close) return false;
		}
		return true;
	}
	
	/// Read a whole parameter, terminated by close
	String readParam(Char close, bool skip_space = true) {
		String ret;
		while(nextUntil(close)) ret += value;
		return ret;
	}
	
	/// Read a parameter, matches nested parentheses, keeps escape sequences
	String readRawParam(Char close1, Char close2 = 0) {
		String ret;
		int parens = 0;
		while (nextUntil(0, false)) {
			if (escaped) {
				ret += _('\\');
			} else {
				if (parens == 0 && (value == close1 || value == close2)) break;
				if      (value == _('(')) parens++;
				else if (value == _(')')) parens--;
			}
			ret += value;
		}
		return ret;
	}
	
	/// Does the current position match a keyword? If so, skip it
	bool keyword(const Char* kw) {
		if (value == kw[0]) {
			if (is_substr(spec, pos, kw + 1)) {
				pos += wxStrlen(kw + 1);
				return true;
			}
		}
		return false;
	}
	
  private:
	const String& spec;
	size_t pos;
};

// ----------------------------------------------------------------------------- : Sort functions

/// Sort a string using a specification using the shortest cycle method, see spec_sort
/** Removed used characters from input! */
void cycle_sort(const String& spec, String& input, String& ret) {
	size_t size = spec.size();
	vector<UInt> counts;
	// count occurences of each char in spec
	FOR_EACH_CONST(s, spec) {
		UInt c = 0;
		FOR_EACH(i, input) {
			if (s == i) {
				i = REMOVED; // remove
				c++;
			}
		}
		counts.push_back(c);
	}
	// determine best start point
	size_t best_start = 0;
	UInt   best_start_score = 0xffffffff;
	for (size_t start = 0 ; start < size ; ++start) {
		// score of a start position, can be considered as:
		//  - count saturated to binary
		//  - rotated left by start
		//  - interpreted as a binary number, but without trailing 0s
		UInt score = 0, mul = 1;
		for (size_t i = 0 ; i < size ; ++i) {
			mul *= 2;
			if (counts[(start + i) % size]) {
				score = score * mul + 1;
				mul = 1;
			}
		}
		if (score < best_start_score) {
			best_start_score = score;
			best_start       = start;
		}
	}
	// return string
	for (size_t i = 0 ; i < size ; ++i) {
		size_t pos = (best_start + i) % size;
		ret.append(counts[pos], spec[pos]);
	}
}

/// Sort a string, keeping the characters in the original order
/** Removed used characters from input! */
void mixed_sort(const String& spec, String& input, String& ret) {
	FOR_EACH(c, input) {
		if (spec.find(c) != String::npos) {
			ret += c;
			c = REMOVED;
		}
	}
}

/// Sort a string, find a compound item
/** Removed used characters from input! */
void compound_sort(const String& spec, String& input, String& ret) {
	size_t pos = input.find(spec);
	while (pos != String::npos) {
		ret += spec;
		for (size_t j = 0 ; j < spec.size() ; ++j) input.SetChar(pos + j, REMOVED);
		pos = input.find(spec, pos + 1);
	}
}

/// Sort things matching a pattern
void pattern_sort(const String& pattern, const String& spec, String& input, String& ret) {
	if (pattern.size() > input.size()) return;
	size_t end = input.size() - pattern.size() + 1;
	for (size_t pos = 0 ; pos < end ; ++pos) {
		// does the pattern match here?
		String placeholders;
		bool match = true;
		for (size_t j = 0 ; j < pattern.size() ; ++j) {
			Char c = input.GetChar(pos + j);
			Char p = pattern.GetChar(j);
			if (c == REMOVED)  { match = false; break; }
			else if (p == PLACEHOLDER) {
				placeholders += c;
			} else if (c != p) { match = false; break; }
		}
		// do we have a match?
		if (match) {
			// sort placeholders
			String new_placeholders = spec_sort(spec, placeholders);
			if (new_placeholders.size() == placeholders.size()) {
				// add to output, erase from input
				size_t ph = 0;
				for (size_t j = 0 ; j < pattern.size() ; ++j) {
					Char p = pattern.GetChar(j);
					if (p == PLACEHOLDER) {
						ret += new_placeholders.GetChar(ph++);
					} else {
						ret += p;
					}
					input.SetChar(pos + j, REMOVED);
				}
				// erase from input
				pos += pattern.size() - 1;
			}
		}
	}
}

/// Sort things in place, keep the rest of the input
void in_place_sort(const String& spec, String& input, String& ret) {
	String result;
	spec_sort(spec, input, result);
	// restore into the same order as in 'input'
	size_t pos_r = 0;
	FOR_EACH(c, input) {
		if (c == REMOVED) {
			if (pos_r < result.size()) {
				ret += result.GetChar(pos_r++);
			}
		} else {
			ret += c;
		}
	}
	input.clear(); // we ate all the input
}

// ----------------------------------------------------------------------------- : spec_sort

String spec_sort(const String& spec, String& input, String& ret) {
	SpecIterator it(spec);
	while(it.nextUntil(0)) {
		if (it.escaped) {					// single character, escaped
			FOR_EACH(d, input) {
				if (d == it.value) {
					ret += d;
					d = REMOVED;
				}
			}
		} else if (it.value == _('<')) {		// keep only a single copy
			while (it.nextUntil(_('>'))) {
				size_t pos = input.find_first_of(it.value);
				if (pos != String::npos) {
					input.SetChar(pos, REMOVED);
					ret += it.value; // input contains it.value
				}
			}
		} else if (it.keyword(_("once("))) {
			while (it.nextUntil(_(')'))) {
				size_t pos = input.find_first_of(it.value);
				if (pos != String::npos) {
					input.SetChar(pos, REMOVED);
					ret += it.value; // input contains it.value
				}
			}
			
		} else if (it.value == _('[')) {	// in input order
			mixed_sort(it.readParam(_(']')), input, ret);
		} else if (it.keyword(_("mixed("))) {
			mixed_sort(it.readParam(_(')')), input, ret);
			
		} else if (it.keyword(_("cycle("))) {
			cycle_sort(it.readParam(_(')')), input, ret);
		} else if (it.value == _('(')) {
			cycle_sort(it.readParam(_(')')), input, ret);
		
		} else if (it.keyword(_("compound("))) { // compound item
			compound_sort(it.readParam(_(')')), input, ret);
		
		} else if (it.keyword(_("pattern("))) { // recurse with pattern
			String pattern;
			// read pattern
			while (it.nextUntil(_(' '), false)) {
				if (it.value == _('.') && !it.escaped) {
					it.value = PLACEHOLDER;
				}
				pattern += it.value;
			}
			// read spec to apply to pattern
			String sub_spec = it.readRawParam(_(')'));
			// sort
			pattern_sort(pattern, sub_spec, input, ret);
		
		} else if (it.keyword(_("in_place("))) { // recurse without pattern
			// read spec to apply to pattern
			String sub_spec = it.readRawParam(_(')'));
			in_place_sort(sub_spec, input, ret);
		
		} else if (it.keyword(_("any()"))) { // remaining input
			FOR_EACH(d, input) {
				if (d != REMOVED) {
					ret += d;
					d = REMOVED;
				}
			}
		
		} else if (it.keyword(_("reverse_order("))) { // reverse order of preference
			size_t old_ret_size = ret.size();
			while (it.value != _(')')) {
				size_t before_ret_size = ret.size();
				String sub_spec = it.readRawParam(_(')'),_(' '));
				spec_sort(sub_spec, input, ret);
				// reverse this item
				reverse(ret.begin() + before_ret_size, ret.end());
			}
			// re-reverse all items
			reverse(ret.begin() + old_ret_size, ret.end());
		
		} else if (it.keyword(_("ordered("))) { // in spec order
			while (it.nextUntil(_(')'))) {
				FOR_EACH(d, input) {
					if (d == it.value) {
						ret += d;
						d = REMOVED;
					}
				}
			}
		} else {					// single char
			FOR_EACH(d, input) {
				if (d == it.value) {
					ret += d;
					d = REMOVED;
				}
			}
		}
	}
	return ret;
}

String spec_sort(const String& spec, String input) {
	String ret;
	spec_sort(spec, input, ret);
	return ret;
}
