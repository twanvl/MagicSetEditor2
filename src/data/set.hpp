//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_SET
#define HEADER_DATA_SET

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/action_stack.hpp>
#include <util/io/package.hpp>
#include <data/field.hpp> // for Set::value
#include <data/keyword.hpp>
#include <boost/scoped_ptr.hpp>

DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
DECLARE_POINTER_TYPE(Styling);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Value);
DECLARE_POINTER_TYPE(Keyword);
DECLARE_POINTER_TYPE(ScriptValue);
class SetScriptManager;
class SetScriptContext;
class Context;
class Dependency;
template <typename> class OrderCache;
typedef intrusive_ptr<OrderCache<CardP> > OrderCacheP;

// ----------------------------------------------------------------------------- : Set

/// A set of cards
class Set : public Packaged {
  public:
	/// Create a set, the set should be open()ed later
	Set();
	/// Create a set using the given game
	Set(const GameP& game);
	/// Create a set using the given stylesheet, and its game
	Set(const StyleSheetP& stylesheet);
	~Set();
  
	GameP game;					///< The game this set uses
	StyleSheetP stylesheet;		///< The default stylesheet
	/// The values on the fields of the set
	/** The indices should correspond to the set_fields in the Game */
	IndexMap<FieldP, ValueP> data;
	/// Extra values for specitic stylesheets, indexed by stylesheet name
	DelayedIndexMaps<FieldP,ValueP> styling_data;
	vector<CardP> cards;		///< The cards in the set
	vector<KeywordP> keywords;	///< Additional keywords used in this set
	String apprentice_code;		///< Code to use for apprentice (Magic only)
	ActionStack actions;		///< Actions performed on this set and the cards in it
	KeywordDatabase keyword_db;	///< Database for matching keywords, must be cleared when keywords change
	
	/// A context for performing scripts
	/** Should only be used from the main thread! */
	Context& getContext();
	/// A context for performing scripts on a particular card
	/** Should only be used from the main thread! */
	Context& getContext(const CardP& card);
	/// Update styles and extra_card_fields for a card
	void updateStyles(const CardP& card);
	/// Update scripts that were delayed
	void updateDelayed();
	/// A context for performing scripts
	/** Should only be used from the thumbnail thread! */
	Context& getContextForThumbnails();
	/// A context for performing scripts on a particular card
	/** Should only be used from the thumbnail thread! */
	Context& getContextForThumbnails(const CardP& card);
	/// A context for performing scripts on a particular stylesheet
	/** Should only be used from the thumbnail thread! */
	Context& getContextForThumbnails(const StyleSheetP& stylesheet);
	
	/// Stylesheet to use for a particular card
	/** card may be null */
	const StyleSheet& stylesheetFor (const CardP& card);
	StyleSheetP       stylesheetForP(const CardP& card);
	
	/// Styling information for a particular stylesheet
	IndexMap<FieldP, ValueP>& stylingDataFor(const StyleSheet&);
	
	/// Find a value in the data by name and type
	template <typename T> T& value(const String& name) {
		for(IndexMap<FieldP, ValueP>::iterator it = data.begin() ; it != data.end() ; ++it) {
			if ((*it)->fieldP->name == name) {
				T* ret = dynamic_cast<T*>(it->get());
				if (!ret) throw InternalError(_("Set field with name '")+name+_("' doesn't have the right type"));
				return *ret;
			}
		}
		throw InternalError(_("Expected a set field with name '")+name+_("'"));
	}
	
	/// Find the position of a card in this set, when the card list is sorted using the given cirterium
	int positionOfCard(const CardP& card, const ScriptValueP& order_by);
	/// Clear the order_cache used by positionOfCard
	void clearOrderCache();
	
	virtual String typeName() const;
	/// Validate that the set is correctly loaded
	virtual void validate(Version = app_version);
	
  private:
	DECLARE_REFLECTION();
	
	/// Object for managing and executing scripts
	scoped_ptr<SetScriptManager> script_manager;
	/// Object for executing scripts from the thumbnail thread
	scoped_ptr<SetScriptContext> thumbnail_script_context;
	/// Cache of cards ordered by some criterion
	map<ScriptValueP,OrderCacheP> order_cache;
};

inline int item_count(const Set& set) {
	return (int)set.cards.size();
}
ScriptValueP make_iterator(const Set& set);

void mark_dependency_member(const Set& set, const String& name, const Dependency& dep);

// ----------------------------------------------------------------------------- : SetView

/// A 'view' of a Set, is notified when the Set is updated
/** To listen to events, derived classes should override onAction(const Action&, bool undone)
 */
class SetView : public ActionListener {
  public:
	SetView();
	~SetView();
	
	/// Get the set that is currently being viewed
	inline SetP getSet() { return set; }
	/// Change the set that is being viewed
	void setSet(const SetP& set);
	
  protected:
	/// The set that is currently being viewed, should not be modified directly!
	SetP set;
	
	/// Called when another set is being viewed (using setSet)
	virtual void onChangeSet() {}
	/// Called when just before another set is being viewed (using setSet)
	virtual void onBeforeChangeSet() {}
};


// ----------------------------------------------------------------------------- : EOF
#endif
