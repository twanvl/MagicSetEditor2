//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/action/keyword.hpp>
#include <data/keyword.hpp>
#include <data/set.hpp>
#include <data/game.hpp>

DECLARE_TYPEOF_COLLECTION(KeywordModeP);

// ----------------------------------------------------------------------------- : Add Keyword

AddKeywordAction::AddKeywordAction(Set& set)
	: KeywordListAction(set), keyword(new Keyword())
{
	// find default mode
	FOR_EACH(mode, set.game->keyword_modes) {
		if (mode->is_default) {
			keyword->mode = mode->name;
			break;
		}
	}
}

AddKeywordAction::AddKeywordAction(Set& set, const KeywordP& keyword)
	: KeywordListAction(set), keyword(keyword)
{}

String AddKeywordAction::getName(bool to_undo) const {
	return _("Add keyword");
}

void AddKeywordAction::perform(bool to_undo) {
	if (!to_undo) {
		set.keywords.push_back(keyword);
	} else {
		assert(!set.keywords.empty());
		set.keywords.pop_back();
	}
}

// ----------------------------------------------------------------------------- : Remove Keyword

RemoveKeywordAction::RemoveKeywordAction(Set& set, const KeywordP& keyword)
	: KeywordListAction(set), keyword(keyword)
	// find the keyword_id of the keyword we want to remove
	, keyword_id(find(set.keywords.begin(), set.keywords.end(), keyword) - set.keywords.begin())
{
	if (keyword_id >= set.keywords.size()) {
		throw InternalError(_("Keyword to remove not found in set"));
	}
}

String RemoveKeywordAction::getName(bool to_undo) const {
	return _("Remove keyword");
}

void RemoveKeywordAction::perform(bool to_undo) {
	if (!to_undo) {
		assert(keyword_id < set.keywords.size());
		set.keywords.erase(set.keywords.begin() + keyword_id);
	} else {
		assert(keyword_id <= set.keywords.size());
		set.keywords.insert(set.keywords.begin() + keyword_id, keyword);
	}
}
