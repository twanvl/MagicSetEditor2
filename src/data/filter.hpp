//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Filters

/// A filter function to determine which items are shown in a list
template <typename T>
class Filter : public IntrusivePtrVirtualBase {
  public:
  typedef intrusive_ptr<T> TP;
  
  virtual ~Filter() {}
  /// Should an object be shown in the list?
  virtual bool keep(T const& x) const {
    return false;
  }
  /// Select objects from a list
  virtual void getItems(vector<TP> const& in, vector<VoidP>& out) const {
    for (typename vector<TP>::const_iterator it = in.begin() ; it != in.end() ; ++it) {
      if (keep(**it)) {
        out.push_back(*it);
      }
    }
  }
};

// ----------------------------------------------------------------------------- : Quick search

struct QuickFilterQuery {
  String type;
  String query;
  bool match(String const& key, String const& x) const {
    return (type.empty() || find_i(key,type) != String::npos) && find_i(x,query) != String::npos;
  }
};

struct QuickFilterPart : QuickFilterQuery  {
  bool need_match = true;
};

/// Parse a quick filter string
vector<QuickFilterPart> parse_quicksearch_query(String const& query);

/// Does the given object match the quick search query?
template <typename T>
bool match_quicksearch_query(vector<QuickFilterPart> const& query, T const& object) {
  for (auto const& part : query) {
    if (object.contains(part) != part.need_match) return false;
  }
  return true;
}

/// A filter function that searches for objects containing a string
template <typename T>
class QuickFilter : public Filter<T> {
public:
  QuickFilter(String const& query) : query(parse_quicksearch_query(query)) {}
  virtual bool keep(T const& x) const {
    return match_quicksearch_query(query, x);
  }
private:
  vector<QuickFilterPart> query;
};

