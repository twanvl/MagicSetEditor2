//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/script_manager.hpp>
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/field.hpp>
#include <util/error.hpp>

typedef map<const StyleSheet*,Context*> Contexts;
typedef IndexMap<FieldP,StyleP> IndexMap_FieldP_StyleP;
typedef IndexMap<FieldP,ValueP> IndexMap_FieldP_ValueP;
DECLARE_TYPEOF(Contexts);
DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(Dependency);
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_StyleP);
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_ValueP);

// initialize functions, from functions.cpp
void init_script_functions(Context& ctx);

// ----------------------------------------------------------------------------- : ScriptManager : initialization

ScriptManager::ScriptManager(Set& set)
	: set(set)
{}

ScriptManager::~ScriptManager() {
	set.actions.removeListener(this);
	// destroy context
	FOR_EACH(sc, contexts) {
		delete sc.second;
	}
	// add as an action listener for the set, so we receive actions
	set.actions.addListener(this);
}

Context& ScriptManager::getContext(const StyleSheetP& stylesheet) {
	assert(wxThread::IsMain()); // only use our contexts from the main thread
	
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
		ctx->setVariable(_("set"),        new_intrusive1<ScriptObject<Set*> >(&set));
		ctx->setVariable(_("game"),       toScript(set.game));
		ctx->setVariable(_("stylesheet"), toScript(stylesheet));
		ctx->setVariable(_("card"),       set.cards.empty() ? script_nil : toScript(set.cards.front())); // dummy value
		ctx->setVariable(_("styling"),    toScript(&set.stylingDataFor(*stylesheet)));
		try {
			// perform init scripts, don't use a scope, variables stay bound in the context
			set.game  ->init_script.invoke(*ctx, false);
			stylesheet->init_script.invoke(*ctx, false);
			// find script dependencies
			initDependencies(*ctx, *set.game);
			initDependencies(*ctx, *stylesheet);
			// apply scripts to everything
			updateAll();
		} catch (Error e) {
			handle_error(e, false, false);
		}		
		// initialize dependencies
		return *ctx;
	}
}
Context& ScriptManager::getContext(const CardP& card) {
	Context& ctx = getContext(set.stylesheetFor(card));
	if (card) {
		ctx.setVariable(_("card"), toScript(card));
	} else {
		ctx.setVariable(_("card"), script_nil);
	}
	return ctx;
}

void ScriptManager::initDependencies(Context& ctx, Game& game) {
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


void ScriptManager::initDependencies(Context& ctx, StyleSheet& stylesheet) {
	if (stylesheet.dependencies_initialized) return;
	stylesheet.dependencies_initialized = true;
	// find dependencies of choice images and other style stuff
	FOR_EACH(s, stylesheet.card_style) {
		s->initDependencies(ctx, Dependency(DEP_STYLE, s->fieldP->index, &stylesheet));
	}
}

// ----------------------------------------------------------------------------- : ScriptManager : updating

void ScriptManager::onAction(const Action& action, bool undone) {
	// TODO
//	TYPE_CASE(action, ValueAction) {
//	}
//	TYPE_CASE(action, CardListAction) {
//	}
}

void ScriptManager::updateStyles(const CardP& card) {
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

void ScriptManager::updateValue(Value& value, const CardP& card) {
	Age starting_age; // the start of the update process
	deque<ToUpdate> to_update;
	// execute script for initial changed value
	value.update(getContext(card));
	// update dependent scripts
	alsoUpdate(to_update, value.fieldP->dependent_scripts, card);
	updateRecursive(to_update, starting_age);
}

void ScriptManager::updateAll() {
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
}

void ScriptManager::updateAllDependend(const vector<Dependency>& dependent_scripts) {
	deque<ToUpdate> to_update;
	Age starting_age;
	alsoUpdate(to_update, dependent_scripts, CardP());
	updateRecursive(to_update, starting_age);
}

void ScriptManager::updateRecursive(deque<ToUpdate>& to_update, Age starting_age) {
//	set->order_cache.clear(); // clear caches before evaluating a round of scripts
	while (!to_update.empty()) {
		updateToUpdate(to_update.front(), to_update, starting_age);
		to_update.pop_front();
	}
}

void ScriptManager::updateToUpdate(const ToUpdate& u, deque<ToUpdate>& to_update, Age starting_age) {
	Age age = u.value->last_script_update;
	if (starting_age < age)  return; // this value was already updated
	Context& ctx = getContext(u.card);
	if (u.value->update(ctx)) {
		// changed, send event
//		ScriptValueEvent change(&*u.card, u.value);
//		set.actions.tellListeners(change);
		// u.value has changed, also update values with a dependency on u.value
		alsoUpdate(to_update, u.value->fieldP->dependent_scripts, u.card);
	}
}

void ScriptManager::alsoUpdate(deque<ToUpdate>& to_update, const vector<Dependency>& deps, const CardP& card) {
	FOR_EACH_CONST(d, deps) {
		switch (d.type) {
			case DEP_SET_FIELD: {
				break;
			} case DEP_CARD_FIELD: {
				break;
			} case DEP_CARDS_FIELD: {
				break;
			} case DEP_STYLE: {
				break;
			} case DEP_CARD_COPY_DEP: {
				break;
			} case DEP_SET_COPY_DEP: {
				break;
			} default:
				assert(false);
		}
		/*
		if (d.type == DependendScript.setField) {
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