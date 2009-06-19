//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
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
	if (!part) return false;
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

void SymbolPartsSelection::clearChildren(SymbolPart* part) {
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) {
			if (selected(p)) selection.erase(p);
			clearChildren(p.get());
		}
	}
}

SymbolShapeP SymbolPartsSelection::getAShape() const {
	FOR_EACH(s, selection) {
		if (s->isSymbolShape()) return static_pointer_cast<SymbolShape>(s);
	}
	return SymbolShapeP();
}

SymbolSymmetryP SymbolPartsSelection::getASymmetry() const {
	FOR_EACH(s, selection) {
		if (s->isSymbolSymmetry()) return static_pointer_cast<SymbolSymmetry>(s);
	}
	return SymbolSymmetryP();
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
	Bounds ab(a); ab.update(b);
	Bounds bc(b); bc.update(c);
	FOR_EACH_CONST(p, parent.parts) {
		bool in_ab = ab.contains(p->bounds);
		bool in_bc = bc.contains(p->bounds);
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
