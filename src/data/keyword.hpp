//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_KEYWORD
#define HEADER_DATA_KEYWORD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/scriptable.hpp>
#include <wx/regex.h>

DECLARE_POINTER_TYPE(KeywordParam);
DECLARE_POINTER_TYPE(KeywordExpansion);
DECLARE_POINTER_TYPE(KeywordMode);

// ----------------------------------------------------------------------------- : Keyword components

/// Parameter type of keywords
class KeywordParam {
  public:
	String         name;		///< Name of the parameter type
	String         description;	///< Description of the type
	String         match;		///< Uncompiled regex
	wxRegEx        matchRe;		///< Regular expression to match
	OptionalScript script;		///< Transformation of the value for showing in the reminder text 
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Keyword mode

/// Information on when and how to use a keyword
class KeywordMode {
	String name;		///< Name of the mode
	String description;	///< Description of the type
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Keyword expansion

/// A way to use a keyword
class KeywordExpansion {
  public:
	String                before;		///< Components before the keyword: parameters and separators (tagged string)
	String                after;		///< Components after the keyword: parameters and separators
	vector<KeywordParamP> parameters;	///< The types of parameters
	wxRegEx               splitter;		///< Regular expression to split/match the components, automatically generated
	StringScript          reminder;		///< Reminder text of the keyword
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Keyword

/// A keyword for a set or a game
class Keyword {
  public:
	String                    keyword;		///< The keyword
	vector<KeywordExpansionP> expansions;	///< Expansions, i.e. ways to use this keyword
	String                    rules;		///< Rules/explanation
	String                    mode;			///< Mode of use, can be used by scripts (only gives the name)
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Using keywords

/// Expand/update all keywords in the given string
String expand_keywords(const String& text);

// ----------------------------------------------------------------------------- : EOF
#endif
