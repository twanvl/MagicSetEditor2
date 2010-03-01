//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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
#include <util/error.hpp>
#include <data/field/text.hpp>
#include <data/action/generic.hpp>

class Set;
DECLARE_POINTER_TYPE(Keyword);
DECLARE_TYPEOF_COLLECTION(GenericAddAction<KeywordP>::Step);

// ----------------------------------------------------------------------------- : Add Keyword

/// An Action the changes the keyword list of a set
class KeywordListAction : public Action {
  public:
	inline KeywordListAction(Set& set) : set(set) {}
	
  protected:
	Set& set; // the set owns this action, so the set will not be destroyed before this
	          // therefore we don't need a smart pointer
};

/// Adding or removing a keyword from a set
class AddKeywordAction : public KeywordListAction {
  public:
	AddKeywordAction(Set& set);
	AddKeywordAction(AddingOrRemoving, Set& set, const KeywordP& keyword);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
	const GenericAddAction<KeywordP> action;
};

// ----------------------------------------------------------------------------- : Changing keywords

/// A FakeTextValue that is used to edit an aspect of a keyword
/** These values can be seen in ValueActions.
 *  Can edit one of:
 *    - the keyword name
 *    - the match string
 *    - reminder text
 */
class KeywordTextValue : public FakeTextValue {
  public:
	KeywordTextValue(const TextFieldP& field, Keyword* keyword, String* underlying, bool editable, bool untagged = false)
		: FakeTextValue(field, underlying, editable, untagged)
		, keyword(*keyword)
	{}
	
	Keyword& keyword;	///< The keyword that is being edited
};

/// A FakeTextValue that is used to edit reminder text scripts
class KeywordReminderTextValue : public KeywordTextValue {
  public:
	KeywordReminderTextValue(Set& set, const TextFieldP& field, Keyword* keyword, bool editable);
	
	String errors; ///< Errors in the script
	Set&   set;    ///< Set this keyword is in (for script checking)
	Keyword& keyword; ///< The keyword we are the reminder text of
	
	/// Try to compile the script
	virtual void store();
	/// Add some tags, so the script looks nice
	virtual void retrieve();
	
	/// Syntax highlight, and store in value
	void highlight(const String& code, const vector<ScriptParseError>& errors);
	
	/// Check the script for errors
	bool checkScript(const ScriptP& script);
};

/// Changing the mode of a keyword
class ChangeKeywordModeAction : public Action {
  public:
	ChangeKeywordModeAction(Keyword& keyword, const String& new_mode);
	
	virtual String getName(bool to_undo) const;
	virtual void   perform(bool to_undo);
	
  //private:
	Keyword& keyword;
	String   mode;
};

// ----------------------------------------------------------------------------- : EOF
#endif
