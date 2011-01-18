//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FILTER
#define HEADER_DATA_FILTER

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

/// Does the given object match the quick search query?
template <typename T>
bool match_quicksearch_query(String const& query, T const& object) {
	bool need_match = true;
	// iterate over the components of the query
	for (size_t i = 0 ; i < query.size() ; ) {
		if (query.GetChar(i) == _(' ')) {
			// skip spaces
			i++;
		} else if (query.GetChar(i) == _('-')) {
			// negate the next query, i.e. match only if it is not on the card
			need_match = !need_match;
			i++;
		} else {
			size_t end, next;
			if (query.GetChar(i) == _('"')) {
				// quoted string, match exactly
				i++;
				end =query.find_first_of(_('"'),i);
				next = min(end,query.size()) + 1;
			} else {
				// single word
				next = end = query.find_first_of(_(' '),i);
			}
			bool match = object.contains(query.substr(i,end-i));
			if (match != need_match) {
				return false;
			}
			need_match = true; // next word is no longer negated
			i = next;
		}
	}
	return true;
}

/// A filter function that searches for objects containing a string
template <typename T>
class QuickFilter : public Filter<T> {
  public:
	using typename Filter<T>::TP;
	QuickFilter(String const& query) : query(query) {}
	virtual bool keep(T const& x) const {
		return match_quicksearch_query(query, x);
	}
  private:
	String query;
};

// ----------------------------------------------------------------------------- : EOF
#endif
