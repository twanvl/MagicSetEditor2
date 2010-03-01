//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_SCRIPT_MANAGER
#define HEADER_SCRIPT_SCRIPT_MANAGER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <util/age.hpp>
#include <script/context.hpp>
#include <script/dependency.hpp>
#include <queue>

class Set;
class Value;
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);

// ----------------------------------------------------------------------------- : SetScriptContext

/// Manager of the script context for a set
class SetScriptContext {
  public:
	SetScriptContext(Set& set);
	virtual ~SetScriptContext();
	
	/// Get a context to use for the set, for a given stylesheet
	Context& getContext(const StyleSheetP&);
	/// Get a context to use for the set, for a given card
	Context& getContext(const CardP&);
	
  protected:
	Set&                            set;		///< Set for which we are managing scripts
	map<const StyleSheet*,Context*> contexts;	///< Context for evaluating scripts that use a given stylesheet
	
	/// Called when a new context for a stylesheet is initialized
	virtual void onInit(const StyleSheetP& stylesheet, Context* ctx) {}
};


// ----------------------------------------------------------------------------- : SetScriptManager

/// Manager of the script context for a set, keeps scripts up to date
/** Whenever there is an action all necessary scripts are executed.
 *  Executes both Value scripts and Style scriptables.
 *
 *  The context contains a normal pointer to the set, not a shared/intrusive_ptr, because the set
 *  itself owns this object.
 */
class SetScriptManager : public SetScriptContext, public ActionListener {
  public:
	SetScriptManager(Set& set);
	~SetScriptManager();
	
	/// Update all styles for a particular card
	void updateStyles(const CardP& card, bool only_content_dependent);
	
	/// Update expensive things that were previously delayed
	void updateDelayed();
	
	/// Update all fields of all cards
	/** Update all set info fields
	 *  Doesn't update styles
	 */
	void updateAll();
	
  private:
	virtual void onInit(const StyleSheetP& stylesheet, Context* ctx);
	
	void initDependencies(Context&, Game&);
	void initDependencies(Context&, StyleSheet&);
	
	/// Update a map of styles
	void updateStyles(Context& ctx, const IndexMap<FieldP,StyleP>& styles, bool only_content_dependent);
	/// Updates scripts, starting at some value
	/** if the value changes any dependend values are updated as well */
	void updateValue(Value& value, const CardP& card);
	// Update all values with a specific dependency
	void updateAllDependend(const vector<Dependency>& dependent_scripts, const CardP& card = CardP());
	
	// Something that needs to be updated
	struct ToUpdate {
		ToUpdate(Value* value, CardP card) : value(value), card(card) {}
		Value* value;  ///< value to update
		CardP  card;   ///< card the value is in, or CadP() if it is not a card field
	};
	/// Update all things in to_update, and things that depent on them, etc.
	/** Only update things that are older than starting_age. */
	void updateRecursive(deque<ToUpdate>& to_update, Age starting_age);
	/// Update a value given by a ToUpdate object, and add things depending on it to to_update
	void updateToUpdate(const ToUpdate& u, deque<ToUpdate>& to_update, Age starting_age);
	/// Schedule all things in deps to be updated by adding them to to_update
	void alsoUpdate(deque<ToUpdate>& to_update, const vector<Dependency>& deps, const CardP& card);
	
	/// Delayed update for (bitmask)...
	enum Delay
	{	DELAY_KEYWORDS = 0x01
	,	DELAY_CARDS    = 0x02
	};
	int delay;
	
  protected:
	/// Respond to actions by updating scripts
	void onAction(const Action&, bool undone);
};

// ----------------------------------------------------------------------------- : EOF
#endif
