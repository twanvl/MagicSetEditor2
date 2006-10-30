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
#include <data/field.hpp>
#include <util/error.hpp>

typedef map<const StyleSheet*,Context*> Contexts;
typedef IndexMap<FieldP,StyleP> IndexMap_FieldP_StyleP;
DECLARE_TYPEOF(Contexts);
DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_NO_REV(IndexMap_FieldP_StyleP);

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
		ctx->setVariable(_("set"),        new_intrusive1<ScriptObject<Set*> >(&set));
		ctx->setVariable(_("game"),       toScript(set.game));
		ctx->setVariable(_("stylesheet"), toScript(stylesheet));
		//ctx->style->object = style;
		//ctx->setVariable(_("styling"), toScript(set->extraStyleData(style)));
		try {
			// perform init scripts
			set.game  ->init_script.invoke(*ctx);
			stylesheet->init_script.invoke(*ctx);
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

// ----------------------------------------------------------------------------- : ScriptManager : dependency handling

void ScriptManager::onAction(const Action& action, bool undone) {
	// TODO
}

void ScriptManager::updateAll() {
	// TODO
}