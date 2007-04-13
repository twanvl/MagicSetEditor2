//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_KEYWORD
#define HEADER_DATA_ACTION_KEYWORD

/** @file data/action/keyword.hpp
 *
 *  Actions operating on Keywords and the keyword list of a set
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>

class Set;
DECLARE_POINTER_TYPE(Keyword);

// ----------------------------------------------------------------------------- : Add Keyword

/// An Action the changes the keyword list of a set
class KeywordListAction : public Action {
  public:
	inline KeywordListAction(Set& set) : set(set) {}
	
  protected:
	Set& set; // the set owns this action, so the set will not be destroyed before this
	          // therefore we don't need a smart pointer
};

/// Adding a new keyword to a set
class AddKeywordAction : public KeywordListAction {
  public:
	AddKeywordAction(Set& set);
	AddKeywordAction(Set& set, const KeywordP& keyword);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	const KeywordP keyword; ///< The new keyword
};

// ----------------------------------------------------------------------------- : Remove Keyword

/// Removing a keyword from a set
class RemoveKeywordAction : public KeywordListAction {
  public:
	RemoveKeywordAction(Set& set, const KeywordP& keyword);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	const KeywordP keyword;		///< The removed keyword
	const size_t keyword_id;	///< Position of the keyword in the set
};

// ----------------------------------------------------------------------------- : Changing keywords

// ----------------------------------------------------------------------------- : EOF
#endif
