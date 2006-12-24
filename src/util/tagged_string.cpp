//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/tagged_string.hpp>
#include <stack>

// ----------------------------------------------------------------------------- : Conversion to/from normal string

String untag(const String& str) {
	bool intag = false;
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (c==_('<')) intag = true;
		if (!intag) ret += c==_('\1') ? _('<') : c;
		if (c==_('>')) intag = false;
	}
	return ret;
}

String untag_no_escape(const String& str) {
	bool intag = false;
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (c==_('<')) intag = true;
		if (!intag) ret += c;
		if (c==_('>')) intag = false;
	}
	return ret;
}

String untag_hide_sep(const String& str) {
	return untag(remove_tag_contents(str,_("<sep-soft")));
}

String escape(const String& str) {
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (c==_('<')) ret += _('\1');
		else           ret += c;
	}
	return ret;
}

String fix_old_tags(const String& str) {
	String ret; ret.reserve(str.size());
	stack<String> tags;
	bool intag = false;
	// invariant : intag => !tags.empty()
	for (size_t i = 0 ; i < str.size() ; ++i) {
		Char c = str.GetChar(i);
		if (is_substr(str, i, _("</>"))) {
			i += 2;
			// old style close tag, replace by the correct tag type
			if (!tags.empty()) {
				// need a close tag?
				if (!tags.top().empty()) {
					ret += _("</") + tags.top() + _(">");
				}
				tags.pop();
			}
			intag = false;
		} else {
			ret += c;
			if (c==_('<')) {
				intag = true;
				tags.push(wxEmptyString);
			} else if (c==_('>') && intag) {
				intag = false;
				if (!starts_with(tags.top(), _("kw")) && !starts_with(tags.top(), _("atom"))) {
					// only keep keyword related stuff
					ret.resize(ret.size() - tags.top().size() - 2); // remove from output
					tags.top() = wxEmptyString;
				}
			} else if (intag) {
				tags.top() += c;
			}
		}
	}
	return ret;
}

// ----------------------------------------------------------------------------- : Finding tags

size_t tag_start(const String& str, size_t pos) {
	size_t start = str.find_last_of(_('<'), pos);
	if (start == String::npos) return String::npos;
	size_t end   = skip_tag(str, start);
	if (end <= pos) return String::npos;
	return start;
}

size_t skip_tag(const String& str, size_t start) {
	if (start >= str.size()) return String::npos;
	size_t end = str.find_first_of(_('>'), start);
	return end == String::npos ? String::npos : end + 1;
}

size_t match_close_tag(const String& str, size_t start) {
	String tag  = tag_type_at(str, start);
	String ctag = _("/") + tag;
	size_t size = str.size();
	int taglevel = 1;
	for (size_t pos = start + tag.size() + 2 ; pos < size ; ++pos) {
		Char c = str.GetChar(pos);
		if (c == _('<')) {
			if (is_substr(str, pos + 1, tag)) {
				++taglevel;
				pos += tag.size() + 1;
			} else if (is_substr(str, pos + 1, ctag)) {
				--taglevel; // close tag
				if (taglevel == 0) return pos;
				pos += ctag.size() + 1;
			}
		}
	}
	return String::npos;
}

size_t last_start_tag_before(const String& str, const String& tag, size_t start) {
	start = min(str.size(), start);
	for (size_t pos = start ; pos > 0 ; --pos) {
		if (is_substr(str, pos - 1, tag)) {
			return pos - 1;
		}
	}
	return String::npos;
}

size_t in_tag(const String& str, const String& tag, size_t start, size_t end) {
	if (start > end) swap(start, end);
	size_t pos  = last_start_tag_before(str, tag, start);
	if (pos == String::npos) return String::npos; // no tag found before start
	size_t posE = match_close_tag(str, pos);
	if (posE < end) return String::npos; // the tag ends before end
	return pos;
}


String tag_at(const String& str, size_t pos) {
	size_t end = str.find_first_of(_(">"), pos);
	if (end == String::npos) return wxEmptyString;
	return str.substr(pos + 1, end - pos - 1);
}

String tag_type_at(const String& str, size_t pos) {
	size_t end = str.find_first_of(_(">-"), pos);
	if (end == String::npos) return wxEmptyString;
	return str.substr(pos + 1, end - pos - 1);
}

String close_tag(const String& tag) {
	if (tag.size() < 1) return _("</>");
	else                return _("</") + tag.substr(1);
}

String anti_tag(const String& tag) {
	if (!tag.empty() && tag.GetChar(0) == _('/')) return _("<")  + tag.substr(1) + _(">");
	else                                          return _("</") + tag           + _(">");
}

// ----------------------------------------------------------------------------- : Cursor position

size_t index_to_cursor(const String& str, size_t index, Movement dir) {
	size_t cursor = 0;
	size_t start = 0, end = 0;
	index = min(index, str.size());
	// find the range [start...end) with the same cursor value, that contains index
	// after the loop, cursor corresponds to index end
	for (size_t i = 0 ; i < str.size() ; ) {
		Char c = str.GetChar(i);
		bool has_width = true;
		if (c == _('<')) {
			// a tag
			if (is_substr(str, i, _("<atom")) || is_substr(str, i, _("<sep"))) {
				// skip tag contents, tag counts as a single 'character'
				i = skip_tag(str, match_close_tag(str, i));
			} else {
				i = skip_tag(str, i);
				end = i;
				has_width = false;
			}
		} else {
			i++;
		}
		if (has_width) {
			cursor++;
			start = end;
			end   = i;
			if (end > index) break;
		}
	}
	if (cursor == 0)       return 0;
	if (dir == MOVE_LEFT)  return cursor - 1;
	if (dir == MOVE_RIGHT) return cursor - (start == index);
	// which is nearer? start or end?
	return cursor - ((int)(index - start) <= (int)(end - index));
}

void cursor_to_index_range(const String& str, size_t cursor, size_t& start, size_t& end) {
	start = end = 0;
	size_t cur = 0;
	size_t i = 0;
	while (cur <= cursor && i < str.size()) {
		Char c = str.GetChar(i);
		bool has_width = true;
		if (c == _('<')) {
			// a tag
			if (is_substr(str, i, _("<atom")) || is_substr(str, i, _("<sep"))) {
				// skip tag contents, tag counts as a single 'character'
				i = skip_tag(str, match_close_tag(str, i));
			} else {
				i = skip_tag(str, i);
				has_width = false;
			}
		} else {
			i++;
		}
		if (has_width) {
			cur++;
			if (cur == cursor) start = i;
		}
	}
	end = min(i, str.size());
	if (cur < cursor) start = end = str.size();
}

size_t cursor_to_index(const String& str, size_t cursor) {
	size_t start, end;
	cursor_to_index_range(str, cursor, start, end);
	// TODO: If at i there is <tag></tag> return a position inside the tags
	// This allows formating to be enabled without a selection
	return start;
}


// ----------------------------------------------------------------------------- : Global operations

String remove_tag(const String& str, const String& tag) {
	if (tag.size() < 1)  return str;
	String ctag = close_tag(tag);
	return remove_tag_exact(remove_tag_exact(str, tag), ctag);
}

String remove_tag_exact(const String& str, const String& tag) {
	String ret; ret.reserve(str.size());
	size_t start = 0, pos = str.find(tag);
	while (pos != String::npos) {
		ret += str.substr(start, pos - start); // before
		// next
		start = skip_tag(str, pos);
		if (start > str.size()) break;
		pos = str.find(tag, start);
	}
	ret += str.substr(start);
	return ret;
}

String remove_tag_contents(const String& str, const String& tag) {
	String ret; ret.reserve(str.size());
	size_t start = 0, pos = str.find(tag);
	while (pos != String::npos) {
		size_t end = match_close_tag(str, pos);
		if (end == String::npos) return ret; // missing close tag
		ret += str.substr(start, pos - start);
		// next
		start = skip_tag(str, end);
		if (start > str.size()) break;
		pos = str.find(tag, start);
	}
	ret += str.substr(start);
	return ret;
}

// ----------------------------------------------------------------------------- : Updates

/// Return all open or close tags in the given range from a string
/** for example:
 *    if close_tags == false, "text<tag>text</tag>text" --> "<tag>"
 *    if close_tags == true,  "text<tag>text</tag>text" --> "</tag>"
 */
String get_tags(const String& str, size_t start, size_t end, bool close_tags) {
	String ret;
	bool intag = false;
	bool keeptag = false;
	for (size_t i = start ; i < end ; ++i) {
		Char c = str.GetChar(i);
		if (c == _('<') && !intag) {
			intag = true;
			// is this tag an open tag?
			if (i + 1 < end && (str.GetChar(i + 1) == _('/')) == close_tags) {
				keeptag = true;
			}
		}
		if (intag && keeptag) ret += c;
		if (c == _('>')) intag = false;
	}
	return ret;
}

String tagged_substr_replace(const String& input, size_t start, size_t end, const String& replacement) {
	assert(start <= end);
	size_t size = input.size();
	String ret; ret.reserve(size + replacement.size() - (end - start)); // estimated size
	return simplify_tagged(
		substr_replace(input, start, end,
			get_tags(input, start, end, true) + // close tags
			escape(replacement) +
			get_tags(input, start, end, false)  // open tags
		));
}


// ----------------------------------------------------------------------------- : Simplification

String simplify_tagged(const String& str) {
	return simplify_tagged_overlap(simplify_tagged_merge(str));
}

// Add a tag to a stack of tags, try to cancel it out
// If </tag> is in stack remove it and returns true
// otherwise appends <tag> and returns fales
// (where </tag> is the negation of tag)
bool add_or_cancel_tag(const String& tag, String& stack) {
	String anti = anti_tag(tag);
	size_t pos = stack.find(anti);
	if (pos == String::npos) {
		stack += _("<") + tag + _(">");
		return false;
	} else {
		// cancel out with anti tag
		stack = stack.substr(0, pos) + stack.substr(pos + anti.size());
		return true;
	}
}

String simplify_tagged_merge(const String& str) {
	String ret; ret.reserve(str.size());
	String waiting_tags; // tags that are waiting to be written to the output
	size_t size = str.size();
	for (size_t i = 0 ; i < size ; ++i) {
		Char c = str.GetChar(i);
		if (c == _('<')) {
			String tag = tag_at(str, i);
			add_or_cancel_tag(tag, waiting_tags);
			i += tag.size() + 1;
		} else {
			ret += waiting_tags;
			waiting_tags.clear();
			ret += c;
		}
	}
	return ret + waiting_tags;
}

String simplify_tagged_overlap(const String& str) {
	String ret;	ret.reserve(str.size());
	String open_tags; // tags we are in
	size_t size = str.size();
	for (size_t i = 0 ; i < size ; ++i) {
		Char c = str.GetChar(i);
		if (c == _('<')) {
			String tag = tag_at(str, i);
			if (starts_with(tag,  _("b")) || starts_with(tag,  _("i")) || starts_with(tag,  _("sym")) ||
			    starts_with(tag, _("/b")) || starts_with(tag, _("/i")) || starts_with(tag, _("/sym"))) {
				// optimize this tag
				if (open_tags.find(_("<") + tag + _(">")) == String::npos) {
					// we are not already inside this tag
					add_or_cancel_tag(tag, open_tags);
					if (open_tags.find(anti_tag(tag)) != String::npos) {
						// still not canceled out
						i += tag.size() + 2;
						continue;
					}
				} else {
					// skip this tag, doubling it has no effect
					i += tag.size() + 2;
					add_or_cancel_tag(tag, open_tags);
					continue;
				}
			}
		}
		ret += c;
	}
	return ret;
}
