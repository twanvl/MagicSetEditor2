//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/script_manager.hpp>
#include <script/to_value.hpp>
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/field.hpp>
#include <data/action/set.hpp>
#include <data/action/value.hpp>
#include <util/error.hpp>

typedef map<const StyleSheet*,Context*> Contexts;
DECLARE_TYPEOF(Contexts);
DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(Dependency);
DECLARE_TYPEOF_NO_REV2(IndexMap<FieldP,StyleP>);
DECLARE_TYPEOF_NO_REV2(IndexMap<FieldP,ValueP>);

// initialize functions, from functions.cpp
void init_script_functions(Context& ctx);
void init_script_image_functions(Context& ctx);

//#define LOG_UPDATES

// ----------------------------------------------------------------------------- : SetScriptContext : initialization

SetScriptContext::SetScriptContext(Set& set)
	: set(set)
{}

SetScriptContext::~SetScriptContext() {
	// destroy contexts
	FOR_EACH(sc, contexts) {
		delete sc.second;
	}
}

Context& SetScriptContext::getContext(const StyleSheetP& stylesheet) {
	Contexts::iterator it = contexts.find(stylesheet.get());
	if (it != contexts.end()) {
		return *it->second; // we already have a context
	} else {
		// create a new context
		Context* ctx = new Context();
		contexts.insert(make_pair(stylesheet.get(), ctx));
		// variables
		//  NOTE: do not use a smart pointer for the pointer to the set, because the set owns this
		//        which would lead to a reference cycle.
		init_script_functions(*ctx);
		init_script_image_functions(*ctx);
		ctx->setVariable(_("set"),        new_intrusive1<ScriptObject<Set*> >(&set));
		ctx->setVariable(_("game"),       toScript(set.game));
		ctx->setVariable(_("stylesheet"), toScript(stylesheet));
		ctx->setVariable(_("card"),       set.cards.empty() ? script_nil : toScript(set.cards.front())); // dummy value
		ctx->setVariable(_("styling"),    toScript(&set.stylingDataFor(*stylesheet)));
		try {
			// perform init scripts, don't use a scope, variables stay bound in the context
			set.game  ->init_script.invoke(*ctx, false);
			stylesheet->init_script.invoke(*ctx, false);
		} catch (const Error& e) {
			handle_error(e, false, false);
		}
		onInit(stylesheet, ctx);
		return *ctx;
	}
}
Context& SetScriptContext::getContext(const CardP& card) {
	Context& ctx = getContext(set.stylesheetFor(card));
	if (card) {
		ctx.setVariable(_("card"), toScript(card));
	} else {
		ctx.setVariable(_("card"), script_nil);
	}
	return ctx;
}

// ----------------------------------------------------------------------------- : SetScriptManager : initialization

SetScriptManager::SetScriptManager(Set& set)
	: SetScriptContext(set)
{
	// add as an action listener for the set, so we receive actions
	set.actions.addListener(this);
}

SetScriptManager::~SetScriptManager() {
	set.actions.removeListener(this);
}

void SetScriptManager::onInit(const StyleSheetP& stylesheet, Context* ctx) {
	assert(wxThread::IsMain());
	// initialize dependencies
	try {
		// find script dependencies
		initDependencies(*ctx, *set.game);
		initDependencies(*ctx, *stylesheet);
	} catch (const Error& e) {
		handle_error(e, false, false);
	}
}

void SetScriptManager::initDependencies(Context& ctx, Game& game) {
	if (game.dependencies_initialized) return;
	game.dependencies_initialized = true;
	// find dependencies of card fields
	FOR_EACH(f, game.card_fields) {
		f->initDependencies(ctx, Dependency(DEP_CARD_FIELD, f->index));
	}
	// find dependencies of set fields
	FOR_EACH(f, game.set_fields) {
		f->initDependencies(ctx, Dependency(DEP_SET_FIELD, f->index));
	}
}


void SetScriptManager::initDependencies(Context& ctx, StyleSheet& stylesheet) {
	if (stylesheet.dependencies_initialized) return;
	stylesheet.dependencies_initialized = true;
	// find dependencies of choice images and other style stuff
	FOR_EACH(s, stylesheet.card_style) {
		s->initDependencies(ctx, Dependency(DEP_STYLE, s->fieldP->index, &stylesheet));
	}
}

// ----------------------------------------------------------------------------- : ScriptManager : updating

void SetScriptManager::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, ValueAction) {
		// find the affected card
		FOR_EACH(card, set.cards) {
			if (card->data.contains(action.valueP)) {
				updateValue(*action.valueP, card);
				return;
			}
		}
		updateValue(*action.valueP, CardP());
	}
	TYPE_CASE_(action, ScriptValueEvent) {
		return; // Don't go into an infinite loop because of our own events
	}
	TYPE_CASE(action, AddCardAction) {
		// update the added card specificly
		Context& ctx = getContext(action.card);
		FOR_EACH(v, action.card->data) {
			v->update(ctx);
		}
		// note: fallthrough
	}
	TYPE_CASE_(action, CardListAction) {
		#ifdef LOG_UPDATES
			wxLogDebug(_("Card dependencies"));
		#endif
		updateAllDependend(set.game->dependent_scripts_cards);
		#ifdef LOG_UPDATES
			wxLogDebug(_("-------------------------------\n"));
		#endif
	}
}

void SetScriptManager::updateStyles(const CardP& card) {
//	lastUpdatedCard = card;
	StyleSheetP stylesheet = set.stylesheetFor(card);
	Context& ctx = getContext(card);
	// update all styles
	FOR_EACH(s, stylesheet->card_style) {
		if (s->update(ctx)) {
			// style has changed, tell listeners
//			ScriptStyleEvent change(s.get());
//			set->actions.tellListeners(change);
		}
	}
}

void SetScriptManager::updateValue(Value& value, const CardP& card) {
	Age starting_age; // the start of the update process
	deque<ToUpdate> to_update;
	// execute script for initial changed value
	value.update(getContext(card));
	#ifdef LOG_UPDATES
		wxLogDebug(_("Start:     %s"), value.fieldP->name);
	#endif
	// update dependent scripts
	alsoUpdate(to_update, value.fieldP->dependent_scripts, card);
	updateRecursive(to_update, starting_age);
	#ifdef LOG_UPDATES
		wxLogDebug(_("-------------------------------\n"));
	#endif
}

void SetScriptManager::updateAll() {
	#ifdef LOG_UPDATES
		wxLogDebug(_("Update all"));
	#endif
	// update set data
	Context& ctx = getContext(set.stylesheet);
	FOR_EACH(v, set.data) {
		v->update(ctx);
	}
	// update card data of all cards
	FOR_EACH(card, set.cards) {
		Context& ctx = getContext(card);
		FOR_EACH(v, card->data) {
			v->update(ctx);
		}
	}
	// update things that depend on the card list
	updateAllDependend(set.game->dependent_scripts_cards);
	#ifdef LOG_UPDATES
		wxLogDebug(_("-------------------------------\n"));
	#endif
}

void SetScriptManager::updateAllDependend(const vector<Dependency>& dependent_scripts) {
	deque<ToUpdate> to_update;
	Age starting_age;
	alsoUpdate(to_update, dependent_scripts, CardP());
	updateRecursive(to_update, starting_age);
}

void SetScriptManager::updateRecursive(deque<ToUpdate>& to_update, Age starting_age) {
	set.clearOrderCache(); // clear caches before evaluating a round of scripts
	while (!to_update.empty()) {
		updateToUpdate(to_update.front(), to_update, starting_age);
		to_update.pop_front();
	}
}

void SetScriptManager::updateToUpdate(const ToUpdate& u, deque<ToUpdate>& to_update, Age starting_age) {
	Age age = u.value->last_script_update;
	if (starting_age < age)  return; // this value was already updated
	Context& ctx = getContext(u.card);
	if (u.value->update(ctx)) {
		// changed, send event
		ScriptValueEvent change(u.card.get(), u.value);
		set.actions.tellListeners(change, false);
		// u.value has changed, also update values with a dependency on u.value
		alsoUpdate(to_update, u.value->fieldP->dependent_scripts, u.card);
	#ifdef LOG_UPDATES
		wxLogDebug(_("Changed: %s"), u.value->fieldP->name);
	#endif
	}
	#ifdef LOG_UPDATES
	else
		wxLogDebug(_("Same:    %s"), u.value->fieldP->name);
	#endif
}

void SetScriptManager::alsoUpdate(deque<ToUpdate>& to_update, const vector<Dependency>& deps, const CardP& card) {
	FOR_EACH_CONST(d, deps) {
		switch (d.type) {
			case DEP_SET_FIELD: {
				ValueP value = set.data.at(d.index);
				to_update.push_back(ToUpdate(value.get(), CardP()));
				break;
			} case DEP_CARD_FIELD: {
				if (card) {
					ValueP value = card->data.at(d.index);
					to_update.push_back(ToUpdate(value.get(), card));
					break;
				} else {
					// There is no card, so the update should affect all cards (fall through).
				}
			} case DEP_CARDS_FIELD: {
				// something invalidates a card value for all cards, so all cards need updating
				FOR_EACH(card, set.cards) {
					ValueP value = card->data.at(d.index);
					to_update.push_back(ToUpdate(value.get(), card));
				}
				break;
			} case DEP_STYLE: {
				// a generated image has become invalid, there is not much we can do
				// because the index is not exact enough, it only gives the field
				StyleSheet* stylesheet = reinterpret_cast<StyleSheet*>(d.data);
				StyleP style = stylesheet->card_style.at(d.index);
				style->invalidate();
				// something changed, send event
				ScriptStyleEvent change(stylesheet, style.get());
				set.actions.tellListeners(change, false);
				break;
			} case DEP_CARD_COPY_DEP: {
				// propagate dependencies from another field
				FieldP f = set.game->card_fields[d.index];
				alsoUpdate(to_update, f->dependent_scripts, card);
				break;
			} case DEP_SET_COPY_DEP: {
				// propagate dependencies from another field
				FieldP f = set.game->set_fields[d.index];
				alsoUpdate(to_update, f->dependent_scripts, card);
				break;
			} default:
				assert(false);
		}
/*		if (d.type == DependendScript.setField) {
			// from set data
			ValueP value = set->data.at(ds.index);
			toUpdate.push_back(ToUpdate(&*value));
		} else if (ds.type == DependendScript.cardField) {
			// from the same card's data
			assert(card);
			ValueP value = card->data.at(ds.index);
			toUpdate.push_back(ToUpdate(&*value, card));
		} else if (ds.type == DependendScript.cardsField) {
			// something invalidates a card value for all cards, so all cards need updating
			FOR_EACH(card, set)->cards {
				ValueP value = card->data.at(ds.index);
				toUpdate.push_back(ToUpdate(&*value, card));
			}
		} else if (ds.type >= DependendScript.choiceImage) {
			// a generated image has become invalid, there is not much we can do
			// because the index is not exact enough, it only gives the field
			// TODO : Indicate what style
			//CardStyleP style = set->styleOf(card) // WRONG?
			CardStyle* style = CardStyle.getByIndex(ds.type - DependendScript.choiceImage);
			StyleP s = style->cardStyle.at(ds.index);
			s->invalidate();
			// something changed, send event
			ScriptStyleEvent change(&*s);
			set->actions.tellListeners(change);
		} else if (ds.type == DependendScript.cardCopyDep) {
			// propagate dependencies from another field
			FieldP f = game->cardFields#(ds.index);
			alsoUpdate(toUpdate, f->dependendScripts, card);
		} else if (ds.type == DependendScript.setCopyDep) {
			// propagate dependencies from another field
			FieldP f = game->setFields#(ds.index);
			alsoUpdate(toUpdate, f->dependendScripts, card);
		} else {
			assert(false); // only setField, cardField and cardsField should be possible
		}*/
	}
}
