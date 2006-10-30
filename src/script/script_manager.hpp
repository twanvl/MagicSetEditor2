//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_SCRIPT_MANAGER
#define HEADER_SCRIPT_SCRIPT_MANAGER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <script/context.hpp>
#include <script/dependency.hpp>

class Set;
class Value;
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
DECLARE_POINTER_TYPE(Card);

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
	~ScriptManager();
		
	/// Get a context to use for the set, for a given stylesheet
	Context& getContext(const StyleSheetP& s);
	
  private:
	Set&                            set;		///< Set for which we are managing scripts
	map<const StyleSheet*,Context*> contexts;	///< Context for evaluating scripts that use a given stylesheet
	
	void initDependencies(Context&, Game&);
	void initDependencies(Context&, StyleSheet&);
	
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
