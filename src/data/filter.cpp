//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/filter.hpp>

// ----------------------------------------------------------------------------- : Quick filter

vector<QuickFilterPart> parse_quicksearch_query(String const& query) {
  // iterate over the components of the query
  vector<QuickFilterPart> parts;
  QuickFilterPart part;
  bool quoted = false;
  for (wxUniChar c : query) {
    if (isSpace(c) && !quoted) {
      if (!part.query.empty()) {
        parts.push_back(part);
        part = {};
      }
    } else if (c == _('"')) {
      // begin/end quoted string, match exactly
      quoted = !quoted;
    } else if (c == _(':') && part.type.empty() && !quoted) {
      part.type = part.query;
      part.query.clear();
    } else if (c == _('-') && part.query.empty() && part.type.empty() && !quoted) {
      // negate
      part.need_match = false;
    } else {
      part.query += c;
    }
  }
  if (!part.query.empty()) {
    parts.push_back(part);
  }
  return parts;
}