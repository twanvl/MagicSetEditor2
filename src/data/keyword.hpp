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
DECLARE_POINTER_TYPE(KeywordMode);
DECLARE_POINTER_TYPE(Keyword);
DECLARE_POINTER_TYPE(ParamReferenceType);
class KeywordTrie;

// ----------------------------------------------------------------------------- : Keyword parameters

class ParamReferenceType {
  public:
	String        name;        ///< Name of the parameter reference type
	String        description; ///< Description (for status bar)
	StringScript  script;      ///< Code to insert into the reminder text script, input is the actual parameter name
	
	DECLARE_REFLECTION();
};

/// Parameter type of keywords
class KeywordParam {
  public:
	KeywordParam();
	String         name;		///< Name of the parameter type
	String         description;	///< Description of the parameter type
	String         placeholder;	///< Placholder for <atom-kwpph>, name is used if this is empty
	bool           optional;	///< Can this parameter be left out (a placeholder is then used)
	String         match;		///< Regular expression to match
	OptionalScript script;		///< Transformation of the value for showing in the reminder text 
	String         example;		///< Example for the keyword editor
	vector<ParamReferenceTypeP> refer_scripts;///< Way to refer to a parameter from the reminder text script
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Keyword mode

/// Information on when and how to use a keyword
class KeywordMode {
  public:
	KeywordMode() : is_default(false) {}
	
	String name;		///< Name of the mode
	String description;	///< Description of the type
	bool   is_default;	///< This is the default mode for new keywords
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Keyword expansion

/// A keyword for a set or a game
class Keyword {
  public:
	Keyword() : fixed(false) {}
	
	String                keyword;		///< The keyword, only for human use
	String                rules;		///< Rules/explanation
	String                match;		///< String to match, <atom-param> tags are used for parameters
	vector<KeywordParamP> parameters;	///< The types of parameters
	StringScript          reminder;		///< Reminder text of the keyword
	String                mode;			///< Mode of use, can be used by scripts (only gives the name)
	/// Regular expression to match and split parameters, automatically generated.
	/** The regex has exactly 2 * parameters.size() + 1 captures (excluding the entire match, caputure 0),
	 *  captures 1,3,... capture the plain text of the match string
	 *  captures 2,4,... capture the parameters
	 */
	wxRegEx               matchRe;
	bool                  fixed;		///< Is this keyword uneditable? (true for game keywods, false for set keywords)
	
	/// Find the index of the mode in a list of possibilities.
	/** Returns the default if not found and 0 if there is no default */
	size_t findMode(const vector<KeywordModeP>& modes) const;
	
	/// Prepare the expansion: (re)generate matchRe and the list of parameters.
	/** Throws when there is an error in the input
	 *  @param param_types A list of all parameter types.
	 *  @param force       Re-prepare even if the regex&parameters are okay
	 */
	void prepare(const vector<KeywordParamP>& param_types, bool force = false);
	
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
	
	/// Prepare the parameters and match regex for a list of keywords
	static void prepare_parameters(const vector<KeywordParamP>&, const vector<KeywordP>&);
	
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
