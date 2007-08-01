//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_SPEC_SORT
#define HEADER_UTIL_SPEC_SORT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : spec_sort

/// Sort a string using a sort specification
/** The specificatio can contain:
 *   - a       = all 'a's go here
 *   - fun(A)  = different behaviour
 *   - [abc]   = mixed(abc)
 *   - <abc>   = once(abc)
 *   - \?      = an escaped ?
 *
 *  'Functions' are:
 *   - ordered(abc) = 'a', 'b' and 'c' go here, in that order
 *   - mixed(abc)   = 'a', 'b' and 'c' go here, in the same order as in the input
 *   - once(abc)    = 'a', 'b' and 'c' go here in that order, and only zero or one time.
 *   - cycle(abc)   = 'a', 'b' and 'c' go here, in the shortest order
 *               consider the specified characters as a clockwise circle
 *               then returns the input in the order that:
 *                 1. takes the shortest clockwise path over this circle.
 *                 2. has _('holes') early, a hole means a character that is in the specification
 *                    but not in the input
 *                 3. prefer the one that comes the earliest in the expression (a in this case)
 *   - compound(abc)    = the connect sting "abc" goes gere
 *   - pattern(.. sort) = sort the things matching the pattern using 'sort' and replace them in the pattern
 *
 *  example:
 *    spec_sort("XYZ<0123456789>cycle(WUBRG)",..)  // used by magic
 *     "W1G")      -> "1GW"      // could be "W...G" or "...GW", second is shorter
 *     "GRBUWWUG") -> "WWUUBRGG" // no difference by rule 1,2, could be "WUBRG", "UBRGW", etc.
 *                               // becomes _("WUBRG") by rule 3
 *     "WUR")      -> "RWU"      // by rule 1 could be "R WU" or "WU R", "RWU" has an earlier hole
 */
String spec_sort(const String& spec, String input);

// ----------------------------------------------------------------------------- : EOF
#endif
