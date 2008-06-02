//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_ACTION_GENERIC
#define HEADER_DATA_ACTION_GENERIC

/** @file data/action/generic.hpp
 *
 *  Generic action stuff
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>

// ----------------------------------------------------------------------------- : Generic add/remove action

enum AddingOrRemoving {ADD, REMOVE};

/// Adding or removing some objects from a vector
template <typename T>
class GenericAddAction {
  public:
	GenericAddAction(AddingOrRemoving, const T& item,          const vector<T>& container);
	GenericAddAction(AddingOrRemoving, const vector<T>& items, const vector<T>& container);
	
	String getName() const;
	void   perform(vector<T>& container, bool to_undo) const;
	
	/// A step of removing/adding
	struct Step {
		inline Step(size_t pos, const T& item) : pos(pos), item(item) {}
		size_t pos;
		T      item;
	};
	bool adding;        ///< Were objects added? (as opposed to removed)
	vector<Step> steps; ///< Added/removed objects, sorted by ascending pos
};

// ----------------------------------------------------------------------------- : Implementation

template <typename T>
bool contains(const vector<T>& items, const T& item) {
	return find(items.begin(), items.end(), item) != items.end();
}

template <typename T>
GenericAddAction<T>::GenericAddAction(AddingOrRemoving ar, const T& item, const vector<T>& container)
	: adding(ar == ADD)
{
	if (ar == ADD) {
		size_t pos = container.size();
		steps.push_back(Step(pos, item));
	} else {
		for (size_t pos = 0 ; pos < container.size() ; ++pos) {
			if (container[pos] == item) {
				steps.push_back(Step(pos, item));
				return;
			}
		}
		throw InternalError(_("Item to remove not found in container"));
	}
}

template <typename T>
GenericAddAction<T>::GenericAddAction(AddingOrRemoving ar, const vector<T>& items, const vector<T>& container)
	: adding(ar == ADD)
{
	if (ar == ADD) {
		size_t pos = container.size();
		for (typename vector<T>::const_iterator it = items.begin() ; it != items.end() ; ++it) {
			steps.push_back(Step(pos++, *it));
		}
	} else {
		for (size_t pos = 0 ; pos < container.size() ; ++pos) {
			if (contains(items, container[pos])) {
				steps.push_back(Step(pos, container[pos]));
			}
		}
		if (steps.size() != items.size()) {
			throw InternalError(_("Item to remove not found in container"));
		}
	}
}

template <typename T>
String GenericAddAction<T>::getName() const {
	String type = type_name(steps.front().item) + (steps.size() == 1 ? _("") : _("s"));
	return adding ? _ACTION_1_("add item", type) : _ACTION_1_("remove item", type);
}

template <typename T>
void GenericAddAction<T>::perform(vector<T>& container, bool to_undo) const {
	if (adding != to_undo) {
		// (re)insert the items
		// ascending order, this is the reverse of removal
		FOR_EACH_CONST(s, steps) {
			assert(s.pos <= container.size());
			container.insert(container.begin() + s.pos, s.item);
		}
	} else {
		// remove the items
		// descending order, because earlier removals shift the rest of the vector
		FOR_EACH_CONST_REVERSE(s, steps) {
			assert(s.pos < container.size());
			container.erase(container.begin() + s.pos);
		}
	}
}

// ----------------------------------------------------------------------------- : EOF
#endif
