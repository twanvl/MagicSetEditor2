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
DECLARE_POINTER_TYPE(Keyword);
class KeywordTrie;

// ----------------------------------------------------------------------------- : Keyword components

/// Parameter type of keywords
class KeywordParam {
  public:
	String         name;		///< Name of the parameter type
	String         description;	///< Description of the type
	String         match;		///< Regular expression to match
	OptionalScript script;		///< Transformation of the value for showing in the reminder text 
	String         example;		///< Example for preview dialog
	
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
	String                match;		///< String to match, <param> tags are used for parameters
	vector<KeywordParamP> parameters;	///< The types of parameters
	StringScript          reminder;		///< Reminder text of the keyword
	String                mode;			///< Mode of use, can be used by scripts (only gives the name)
	/// Regular expression to match and split parameters, automatically generated.
	/** The regex has exactly 2 * parameters.size() + 1 captures (excluding the entire match, caputure 0),
	 *  captures 1,3,... capture the plain text of the match string
	 *  captures 2,4,... capture the parameters
	 */
	wxRegEx               matchRe;
//%	. Default is the mode of the Keyword.
	
	/// Prepare the expansion: (re)generate matchRe and the list of parameters.
	/** Throws when there is an error in the input
	 *  @param param_types A list of all parameter types.
	 *  @param force       Re-prepare even if the regex&parameters are okay
	 */
	void prepare(const vector<KeywordParamP>& param_types, bool force = false);
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Keyword

/// A keyword for a set or a game
class Keyword {
  public:
	String                    keyword;		///< The keyword
	vector<KeywordExpansionP> expansions;	///< Expansions, i.e. ways to use this keyword
	String                    rules;		///< Rules/explanation
//	String                    mode;			///< Mode of use, can be used by scripts (only gives the name)
	
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : Using keywords

/// A database of keywords to allow for fast matching
/** NOTE: keywords may not be altered after they are added to the database,
 *  The database should be rebuild.
 */
class KeywordDatabase {
  public:
	KeywordDatabase();
	~KeywordDatabase();
	
	/// Add a list of keywords to be matched
	void add(const vector<KeywordP>&);
	/// Add a keyword to be matched
	void add(const Keyword&);
	/// Add an expansion of a keyword to be matched
	void add(const KeywordExpansion&);
	
	/// Prepare the parameters and match regex for a list of keywords
	static void prepare_parameters(const vector<KeywordParamP>&, const vector<KeywordP>&);
	static void prepare_parameters(const vector<KeywordParamP>&, const Keyword&);
	
	/// Clear the database
	void clear();
	/// Is the database empty?
	inline bool empty() const { return !root; }
	
	/// Expand/update all keywords in the given string.
	/** @param expand_default script function indicating whether reminder text should be shown by default
	 *  @param combine_script script function to combine keyword and reminder text in some way
	 *  @param ctx            context for evaluation of scripts
	 */
	String expand(const String& text, const ScriptValueP& expand_default, const ScriptValueP& combine_script, Context& ctx) const;
	
  private:
	KeywordTrie* root;	///< Data structure for finding keywords
};

// ----------------------------------------------------------------------------- : EOF
#endif
