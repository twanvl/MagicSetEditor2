//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_TAGGED_STRING
#define HEADER_UTIL_TAGGED_STRING

/** @file util/tagged_string.hpp
 *
 *  @brief Utility for working with strings with tags
 */
 
// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Conversion to/from normal string

/// Remove all tags from a string and convert escaped '<' back to normal '<'
/** e.g. "<sym>R</> something <i>(note)</>"
 *  becomes "R something (note)"
 */
String untag(const String&);

/// Remove all tags from a string, but doesn't convert back escape codes
/** untag_no_escape(x) = escape(untag(x)) */
String untag_no_escape(const String&);

/// Remove all tags from a string
/** In addition to what untag() does this function also removes <sep-soft>...</sep-soft>
 */
String untag_hide_sep(const String&);

/// Escapes a String by converting '>' to '\1'
String escape(const String&);

/// Convert old style </> close tags to new style </tag> tags
/** This style was used until 0.2.6 */
String fix_old_tags(const String&);

// ----------------------------------------------------------------------------- : Finding tags

/// Returns the position just beyond the tag starting at start
size_t skip_tag(const String& str, size_t start);

/// Find the position of the closing tag matching the tag at start
/** If not found returns String::npos
 */
size_t match_close_tag(const String& str, size_t start);

/// Return the tag at the given position (without the <>)
String tag_at(const String& str, size_t pos);

/// Return the tag at the given position (without the <>)
/** stops at '-', so for "<kw-a>" returns "kw" */
String tag_type_at(const String& str, size_t pos);

/// Matching close tag for a tag
String close_tag(const String& tag);

/// The matching close tag for an open tag and vice versa
String anti_tag(const String& tag);

// ----------------------------------------------------------------------------- : Global operations

/// Remove all instances of a tag and its close tag, but keep the contents.
/** tag doesn't have to be a complete tag, for example remove_tag(str, "<kw-")
 *  removes all tags starting with "<kw-".
 */
String remove_tag(const String& str, const String& tag);

/// Remove all instances of tags starting with tag
String remove_tag_exact(const String& str, const String& tag);

/// Remove all instances of a tag (including contents) from a string
/** tag doesn't have to be a complete tag, for example remove_tag_contents(str, "<kw-")
 *  removes all tags starting with "<kw-".
 */
String remove_tag_contents(const String& str, const String& tag);

// ----------------------------------------------------------------------------- : Simplification

/// Verify that a string is correctly tagged, if it is not, change it so it is
/** Ensures that:
 *   - All tags end, i.e. for each '<' there is a matching '>'
 *   - There are no tags containing '<'
 */
String verify_tagged(const String& str);

/// Simplify a tagged string
/**   - merges adjecent open/close tags: "<tag></tag>" --> ""
 *    - removes overlapping tags: "<i>a<i>b</i>c</i>" --> "<i>abc</i>"
 */
String simplify_tagged(const String& str);

/// Simplify a tagged string by merging adjecent open/close tags "<tag></tag>" --> ""
String simplify_tagged_merge(const String& str);

/// Simplify overlapping formatting tags
String simplify_tagged_overlap(const String& str);

// ----------------------------------------------------------------------------- : EOF
#endif
