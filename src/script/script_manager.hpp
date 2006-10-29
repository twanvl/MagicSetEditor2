//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_SCRIPT_MANAGER
#define HEADER_SCRIPT_SCRIPT_MANAGER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/context.hpp>
#include <data/set.hpp>

// ----------------------------------------------------------------------------- : Dependency

/// Types of dependencies
enum DependencyType
{	DEP_CARD_FIELD			///< dependency of a script in a "card" field
,	DEP_CARDS_FIELD			///< dependency of a script in a "card"  field for all cards
,	DEP_SET_INFO_FIELD		///< dependency of a script in a "set"   field
,	DEP_STYLESHEET_FIELD	///< dependency of a script in a "style" property
,	DEP_CARD_COPY_DEP		///< copy the dependencies from a card field
,	DEP_SET_COPY_DEP		///< copy the dependencies from a set  field
,	DEP_CHOICE_IMAGE		///< dependency of a generated choice image, index2 gives the index of the choice image
};

/// A 'pointer' to some script that depends on another script
class Dependency {
  public:
	DependencyType type   : 4;	///< Type of the dependent script
	UInt           index2 : 10;	///< A second index, used for some types
	size_t         index;		///< index into an IndexMap
};

// ----------------------------------------------------------------------------- : Dependencies of data type members



// ----------------------------------------------------------------------------- : ScriptManager

/// Manager of the script context for a set, keeps scripts up to date
/** Whenever there is an action all necessary scripts are executed.
 *  Executes both Value scripts and Style scriptables.
 *
 *  The context contains a normal pointer to the set, not a shared_ptr, because the set
 *  itself owns this object.
 */
class ScriptManager : public ActionListener {
  public:
	ScriptManager(Set& set);
	
  private:
	Context context; ///< Context for evaluating scripts
	
	void initScriptStuff();
	void initDependencies();
	void initDependencies(const StyleSheetP&);
	void initContext(const StyleSheetP&);
	
	// Update all styles for a particular card
	void updateStyles(const CardP& card);
	/// Updates scripts, starting at some value
	/** if the value changes any dependend values are updated as well */
	void updateValue(Value* value, const CardP& card);
	/// Update all fields of all cards
	/** Update all set info fields
	 *  Doesn't update styles
	 */
	void updateAll();
	// Update all values with a specific dependency
	void updateAllDependend(const vector<Dependency>& dependendScripts);
	
  protected:
	/// Respond to actions by updating scripts
	void onAction(const Action&, bool undone);
};

// ----------------------------------------------------------------------------- : EOF
#endif
