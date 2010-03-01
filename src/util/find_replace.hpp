//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_FIND_REPLACE
#define HEADER_UTIL_FIND_REPLACE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/fdrepdlg.h>

DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(TextValue);

// ----------------------------------------------------------------------------- : Search/replace

/// Information for search/replace
class FindInfo {
  public:
	FindInfo(wxFindReplaceData& what) : what(what) {}
	virtual ~FindInfo() {}
	
	/// Handle that a match was found.
	/** Returns true if we are done and searching should be ended. */
	virtual bool handle(const CardP& card, const TextValueP& value, size_t pos, bool was_selection) = 0;
	/// Should the found text be selected?
	virtual bool select() const { return true; }
	/// Should the current selection also be searched?
	virtual bool searchSelection() const { return false; }
	
	/// Searching forward?
	inline bool forward()       const { return what.GetFlags() & wxFR_DOWN; }
	/// Match whole words?
	inline bool wholeWord()     const { return what.GetFlags() & wxFR_WHOLEWORD; }
	/// Case sensitive?
	inline bool caseSensitive() const { return what.GetFlags() & wxFR_MATCHCASE; }
	/// String to look for
	inline const String& findString() const { return what.GetFindString(); }
	
  protected:
	wxFindReplaceData& what; ///< What to search for, the direction to search in
};

// ----------------------------------------------------------------------------- : EOF
#endif
