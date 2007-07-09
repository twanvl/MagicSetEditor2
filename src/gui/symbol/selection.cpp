//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/selection.hpp>
#include <data/symbol.hpp>
#include <gfx/bezier.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Selection

void SymbolPartsSelection::setSymbol(const SymbolP& symbol) {
	root = symbol.get();
	clear();
}

void SymbolPartsSelection::clear() {
	selection.clear();
}

bool SymbolPartsSelection::select(const SymbolPartP& part, SelectMode mode) {
	assert(part);
	// make sure part is not the decendent of a part that is already selected
	if (mode != SELECT_OVERRIDE) {
		FOR_EACH(s, selection) {
			if (s != part && s->isAncestor(*part)) return false;
		}
	}
	// select
	if (mode == SELECT_OVERRIDE) {
		if (selection.size() == 1 && *selection.begin() == part) return false; // already selected
		selection.clear();
		selection.insert(part);
	} else if (mode == SELECT_IF_OUTSIDE) {
		if (selected(part)) {
			return false;
		} else {
			selection.clear();
			selection.insert(part);
		}
	} else {
		if (selected(part)) {
			selection.erase(part);
		} else {
			selection.insert(part);
			// make part is not the ancestor of a part that is already selected
			clearChildren(part.get());
		}
	}
	return true;
}

SymbolShapeP SymbolPartsSelection::getAShape() const {
	FOR_EACH(s, selection) {
		if (s->isSymbolShape()) return static_pointer_cast<SymbolShape>(s);
	}
	return SymbolShapeP();
}

void SymbolPartsSelection::clearChildren(SymbolPart* part) {
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) {
			if (selected(p)) selection.erase(p);
			clearChildren(p.get());
		}
	}
}


// ----------------------------------------------------------------------------- : Position based

SymbolPartP SymbolPartsSelection::find(const SymbolPartP& part, const Vector2D& pos) const {
	if (SymbolShape* s = part->isSymbolShape()) {
		if (point_in_shape(pos, *s)) return part;
	}
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) {
			SymbolPartP found = find(p, pos);
			if (found) {
				if (part->isSymbolSymmetry() || selected(found)) {
					return found;
				} else {
					return part; // don't select inside groups
				}
			}
		}
	}
	return SymbolPartP();
}

SymbolPartP SymbolPartsSelection::find(const Vector2D& position) const {
	FOR_EACH(p, root->parts) {
		SymbolPartP found = find(p, position);
		if (found) return found;
	}
	return SymbolPartP();
}

// ----------------------------------------------------------------------------- : Rectangle based

bool SymbolPartsSelection::selectRect(const Vector2D& a, const Vector2D& b, const Vector2D& c) {
	return selectRect(*root, a, b, c);
}
bool SymbolPartsSelection::selectRect(const SymbolGroup& parent, const Vector2D& a, const Vector2D& b, const Vector2D& c) {
	bool changes = false;
	FOR_EACH_CONST(p, parent.parts) {
		bool in_ab = (p->min_pos.x >= min(a.x, b.x) && p->min_pos.y >= min(a.y, b.y) && p->max_pos.x <= max(a.x, b.x) && p->max_pos.y <= max(a.y, b.y));
		bool in_bc = (p->min_pos.x >= min(a.x, c.x) && p->min_pos.y >= min(a.y, c.y) && p->max_pos.x <= max(a.x, c.x) && p->max_pos.y <= max(a.y, c.y));
		if (in_ab != in_bc) {
			select(p, SELECT_TOGGLE);
			changes = true;
		} else if (SymbolGroup* g = p->isSymbolGroup()) {
			if (p->isSymbolSymmetry() || selected(p)) {
				selectRect(*g, a, b, c);
			}
		}
	}
	return changes;
}
