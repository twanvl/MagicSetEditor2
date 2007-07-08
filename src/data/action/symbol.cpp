//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/action/symbol.hpp>
#include <data/action/symbol_part.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(pair<SymbolShapeP COMMA SymbolShapeCombine>);
DECLARE_TYPEOF_COLLECTION(pair<SymbolPartP  COMMA size_t            >);
DECLARE_TYPEOF_COLLECTION(SymbolPartP);
DECLARE_TYPEOF_COLLECTION(ControlPointP);

// ----------------------------------------------------------------------------- : Utility

String action_name_for(const set<SymbolPartP>& parts, const String& action) {
	return format_string(action, parts.size() == 1 ? _TYPE_("shape") : _TYPE_("shapes"));
}

// ----------------------------------------------------------------------------- : Moving symbol parts

SymbolPartMoveAction::SymbolPartMoveAction(const set<SymbolPartP>& parts, const Vector2D& delta)
	: parts(parts)
	, delta(delta), moved(-delta)
	, min_pos(Vector2D::infinity()), max_pos(-Vector2D::infinity())
	, constrain(false)
	, snap(0)
{
	// Determine min/max_pos
	FOR_EACH(p, parts) {
		if (SymbolShape* s = p->isSymbolShape()) {
			min_pos = piecewise_min(min_pos, s->min_pos);
			max_pos = piecewise_max(max_pos, s->max_pos);
		}
	}
}

String SymbolPartMoveAction::getName(bool to_undo) const {
	return action_name_for(parts, _ACTION_("move"));
}

void SymbolPartMoveAction::perform(bool to_undo) {
	// move the points back
	FOR_EACH(p, parts) {
		if (SymbolShape* s = p->isSymbolShape()) {
			s->min_pos -= moved;
			s->max_pos -= moved;
			FOR_EACH(pnt, s->points) {
				pnt->pos -= moved;
			}
		} else if (SymbolSymmetry* s = p->isSymbolSymmetry()) {
			s->center -= moved;
		} else {
			throw InternalError(_("Invalid symbol part type"));
		}
	}
	moved = -moved;
}

void SymbolPartMoveAction::move(const Vector2D& deltaDelta) {
	delta += deltaDelta;
	// Determine actual delta, possibly constrained and snapped
	Vector2D d = constrain_snap_vector_offset(min_pos, max_pos, delta, constrain, snap);
	Vector2D dd = d - moved; // move this much more
	// Move each point by d
	moved = -dd;
	perform(false); // (ab)use perform to move by +dd
	moved = d;
}

// ----------------------------------------------------------------------------- : Rotating symbol parts

SymbolPartMatrixAction::SymbolPartMatrixAction(const set<SymbolPartP>& parts, const Vector2D& center)
	: parts(parts)
	, center(center)
{}

void SymbolPartMatrixAction::transform(const Vector2D& mx, const Vector2D& my) {
	// Transform each point
	FOR_EACH(p, parts) {
		if (SymbolShape* s = p->isSymbolShape()) {
			FOR_EACH(pnt, s->points) {
				pnt->pos         = (pnt->pos - center).mul(mx,my) + center;
				pnt->delta_before = pnt->delta_before.mul(mx,my);
				pnt->delta_after  = pnt->delta_after .mul(mx,my);
			}
			// bounds change after transforming
			s->calculateBounds();
		} else if (SymbolSymmetry* s = p->isSymbolSymmetry()) {
			s->handle = s->handle.mul(mx,my);
		} else {
			throw InternalError(_("Invalid symbol part type"));
		}
	}
}


SymbolPartRotateAction::SymbolPartRotateAction(const set<SymbolPartP>& parts, const Vector2D& center)
	: SymbolPartMatrixAction(parts, center)
	, angle(0)
	, constrain(false)
{}

String SymbolPartRotateAction::getName(bool to_undo) const {
	return action_name_for(parts, _ACTION_("rotate"));
}

void SymbolPartRotateAction::perform(bool to_undo) {
	// move the points back
	rotateBy(-angle);
	angle = -angle;
}

void SymbolPartRotateAction::rotateTo(double newAngle) {
	double oldAngle = angle;
	angle = newAngle;
	// constrain?
	if (constrain) {
		// multiples of 2pi/24 i.e. 24 stops
		double mult = (2 * M_PI) / 24;
		angle = floor(angle / mult + 0.5) * mult;
	}
	if (oldAngle != angle) rotateBy(angle - oldAngle);
}

void SymbolPartRotateAction::rotateBy(double deltaAngle) {
	// Rotation 'matrix'
	transform(
		Vector2D(cos(deltaAngle), -sin(deltaAngle)),
		Vector2D(sin(deltaAngle),  cos(deltaAngle))
	);
}


// ----------------------------------------------------------------------------- : Shearing symbol parts

SymbolPartShearAction::SymbolPartShearAction(const set<SymbolPartP>& parts, const Vector2D& center)
	: SymbolPartMatrixAction(parts, center)
//	, constrain(false)
	, snap(0)
{}

String SymbolPartShearAction::getName(bool to_undo) const {
	return action_name_for(parts, _ACTION_("shear"));
}

void SymbolPartShearAction::perform(bool to_undo) {
	// move the points back
	// the vector shear = (x,y) is used as:
	//  <1 x>
	//  <y 1>
	// inverse is:
	//  <1  -x>  /
	//  <-y  1> / (1 - xy)
	// we have: xy = 0 => (1 - xy) = 1
	shearBy(-moved);
}

void SymbolPartShearAction::move(const Vector2D& deltaShear) {
	shear += deltaShear;
	Vector2D d = snap_vector(shear - moved, snap);
	shearBy(d);
	moved += d;
}

void SymbolPartShearAction::shearBy(const Vector2D& shear) {
	// Shear 'matrix'
	transform(
		Vector2D(1,       shear.x),
		Vector2D(shear.y, 1)
	);
}


// ----------------------------------------------------------------------------- : Scaling symbol parts


SymbolPartScaleAction::SymbolPartScaleAction(const set<SymbolPartP>& parts, int scaleX, int scaleY)
	: parts(parts)
	, scaleX(scaleX), scaleY(scaleY)
	, constrain(false)
	, snap(0)
{
	// Find min and max coordinates
	old_min = Vector2D( 1e6, 1e6);
	Vector2D old_max  (-1e6,-1e6);
	FOR_EACH(p, parts) {
		if (SymbolShape* s = p->isSymbolShape()) {
			old_min = piecewise_min(old_min, s->min_pos);
			old_max = piecewise_max(old_max, s->max_pos);
		}
	}
	// new == old
	new_min  = new_real_min  = old_min;
	new_size = new_real_size = old_size = old_max - old_min;
}

String SymbolPartScaleAction::getName(bool to_undo) const {
	return action_name_for(parts, _ACTION_("scale"));
}

void SymbolPartScaleAction::perform(bool to_undo) {
	swap(old_min,  new_min);
	swap(old_size, new_size);
	transformAll();
}

void SymbolPartScaleAction::move(const Vector2D& delta_min, const Vector2D& delta_max) {
	new_real_min  += delta_min;
	new_real_size += delta_max - delta_min;
	update();
}

void SymbolPartScaleAction::update() {
	// Move each point so the range [old_min...old_max] maps to [new_min...new_max]
	// we have already moved to the current [new_min...new_max]
	Vector2D tmp_min = old_min, tmp_size = old_size; // the size before any scaling
	         old_min = new_min; old_size = new_size; // the size before this move
	// the size after the move
	new_min = new_real_min; new_size = new_real_size;
	if (constrain && scaleX != 0 && scaleY != 0) {
		Vector2D scale = new_size.div(tmp_size);
		scale = constrain_vector(scale, true, true);
		new_size = tmp_size.mul(scale);
		new_min += (new_real_size - new_size).mul(Vector2D(scaleX == -1 ? 1 : 0, scaleY == -1 ? 1 : 0));
		// TODO : snapping
	} else if (snap >= 0) {
		if (scaleX + scaleY < 0) {
			new_min = snap_vector(new_min, snap);
			new_size += new_real_min - new_min;
		} else {
			Vector2D new_max = snap_vector(new_min + new_size, snap);
			new_size = new_max - new_min;
		}
	}
	// now move all points
	transformAll();
	// restore old_min/size
	old_min = tmp_min;  old_size = tmp_size;
}

void SymbolPartScaleAction::transformAll() {
	Vector2D scale = new_size.div(old_size);
	FOR_EACH(p, parts) {
		if (SymbolShape* s = p->isSymbolShape()) {
			s->min_pos = transform(s->min_pos);
			s->max_pos = transform(s->max_pos);
			// make sure that max >= min
			if (s->min_pos.x > s->max_pos.x) swap(s->min_pos.x, s->max_pos.x);
			if (s->min_pos.y > s->max_pos.y) swap(s->min_pos.y, s->max_pos.y);
			// scale all points
			FOR_EACH(pnt, s->points) {
				pnt->pos = transform(pnt->pos);
				// also scale handles
				pnt->delta_before = pnt->delta_before.mul(scale);
				pnt->delta_after  = pnt->delta_after .mul(scale);
			}
		} else if (SymbolSymmetry* s = p->isSymbolSymmetry()) {
			throw "TODO";
		} else {
			throw InternalError(_("Invalid symbol part type"));
		}
	}
}

Vector2D SymbolPartScaleAction::transform(const Vector2D& v) {
	// TODO: prevent div by 0
	return (v - old_min).div(old_size).mul(new_size) + new_min;
}

// ----------------------------------------------------------------------------- : Change combine mode

CombiningModeAction::CombiningModeAction(const set<SymbolPartP>& parts, SymbolShapeCombine mode) {
	FOR_EACH(p, parts) {
		if (p->isSymbolShape()) {
			this->parts.push_back(make_pair(static_pointer_cast<SymbolShape>(p),mode));
		}
	}
}

String CombiningModeAction::getName(bool to_undo) const {
	return _ACTION_("change combine mode");
}

void CombiningModeAction::perform(bool to_undo) {
	FOR_EACH(pm, parts) {
		swap(pm.first->combine, pm.second);
	}
}

// ----------------------------------------------------------------------------- : Change name

SymbolPartNameAction::SymbolPartNameAction(const SymbolPartP& part, const String& name)
	: part(part), part_name(name)
{}

String SymbolPartNameAction::getName(bool to_undo) const {
	return _ACTION_("change shape name");
}

void SymbolPartNameAction::perform(bool to_undo) {
	swap(part->name, part_name);
}

// ----------------------------------------------------------------------------- : Add symbol part

AddSymbolPartAction::AddSymbolPartAction(Symbol& symbol, const SymbolPartP& part)
	: symbol(symbol), part(part)
{}

String AddSymbolPartAction::getName(bool to_undo) const {
	return format_string(_ACTION_("add"), part->name);
}

void AddSymbolPartAction::perform(bool to_undo) {
	if (to_undo) {
		assert(!symbol.parts.empty());
		symbol.parts.erase (symbol.parts.begin());
	} else {
		symbol.parts.insert(symbol.parts.begin(), part);
	}
}

// ----------------------------------------------------------------------------- : Remove symbol part

RemoveSymbolPartsAction::RemoveSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts)
	: symbol(symbol)
{
	size_t index = 0;
	FOR_EACH(p, symbol.parts) {
		if (parts.find(p) != parts.end()) {
			removals.push_back(make_pair(p, index)); // remove this part
		}
		++index;
	}
}

String RemoveSymbolPartsAction::getName(bool to_undo) const {
	return format_string(_ACTION_("remove parts"), removals.size() == 1 ? _TYPE_("shape") : _TYPE_("shapes"));
}

void RemoveSymbolPartsAction::perform(bool to_undo) {
	if (to_undo) {
		// reinsert the parts
		// ascending order, this is the reverse of removal
		FOR_EACH(r, removals) {
			assert(r.second <= symbol.parts.size());
			symbol.parts.insert(symbol.parts.begin() + r.second, r.first);
		}
	} else {
		// remove the parts
		// descending order, because earlier removals shift the rest of the vector
		FOR_EACH_REVERSE(r, removals) {
			assert(r.second < symbol.parts.size());
			symbol.parts.erase(symbol.parts.begin() + r.second);
		}
	}
}

// ----------------------------------------------------------------------------- : Duplicate symbol parts

DuplicateSymbolPartsAction::DuplicateSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts)
	: symbol(symbol)
{
	UInt index = 0;
	FOR_EACH(p, symbol.parts) {
		index += 1;
		if (parts.find(p) != parts.end()) {
			// duplicate this part
			duplications.push_back(make_pair(p->clone(), index));
			index += 1; // the clone also takes up space on the vector
		}
	}
}

String DuplicateSymbolPartsAction::getName(bool to_undo) const {
	return format_string(_ACTION_("duplicate"), duplications.size() == 1 ? _TYPE_("shape") : _TYPE_("shapes"));
}

void DuplicateSymbolPartsAction::perform(bool to_undo) {
	if (to_undo) {
		// remove the clones
		// walk in reverse order, otherwise we will shift the vector
		FOR_EACH_REVERSE(d, duplications) {
			assert(d.second < symbol.parts.size());
			symbol.parts.erase(symbol.parts.begin() + d.second);
		}
	} else {
		// insert the clones
		FOR_EACH(d, duplications) {
			assert(d.second <= symbol.parts.size());
			symbol.parts.insert(symbol.parts.begin() + d.second, d.first);
		}
	}
}

void DuplicateSymbolPartsAction::getParts(set<SymbolPartP>& parts) {
	parts.clear();
	FOR_EACH(d, duplications) {
		parts.insert(d.first);
	}
}

// ----------------------------------------------------------------------------- : Reorder symbol parts

ReorderSymbolPartsAction::ReorderSymbolPartsAction(Symbol& symbol, size_t part_id1, size_t part_id2)
	: symbol(symbol), part_id1(part_id1), part_id2(part_id2)
{}

String ReorderSymbolPartsAction::getName(bool to_undo) const {
	return _ACTION_("reorder parts");
}

void ReorderSymbolPartsAction::perform(bool to_undo) {
	assert(part_id1 < symbol.parts.size());
	assert(part_id2 < symbol.parts.size());
	swap(symbol.parts[part_id1], symbol.parts[part_id2]);
}

// ----------------------------------------------------------------------------- : Group symbol parts

GroupSymbolPartsActionBase::GroupSymbolPartsActionBase(Symbol& symbol)
	: symbol(symbol)
{}
void GroupSymbolPartsActionBase::perform(bool to_undo) {
	swap(symbol.parts, old_part_list);
}

GroupSymbolPartsAction::GroupSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts)
	: GroupSymbolPartsActionBase(symbol)
{
	// group parts in the old parts list
	bool done = false;
	SymbolGroupP group(new SymbolGroup);
	group->name = _("Group");
	FOR_EACH(p, symbol.parts) {
		if (parts.find(p) != parts.end()) {
			group->parts.push_back(p);
			if (!done) {
				done = true;
				old_part_list.push_back(group);
			}
		} else {
			// not affected
			old_part_list.push_back(p);
		}
	}
}
String GroupSymbolPartsAction::getName(bool to_undo) const {
	return _ACTION_("group parts");
}

UngroupSymbolPartsAction::UngroupSymbolPartsAction(Symbol& symbol, const set<SymbolPartP>& parts)
	: GroupSymbolPartsActionBase(symbol)
{
	// break up the parts in the old parts list
	FOR_EACH(p, symbol.parts) {
		if (parts.find(p) != parts.end() && p->isSymbolGroup()) {
			// break up the group
			SymbolGroup* g = p->isSymbolGroup();
			FOR_EACH(p, g->parts) {
				old_part_list.push_back(p);
			}
		} else {
			// not affected
			old_part_list.push_back(p);
		}
	}
}
String UngroupSymbolPartsAction::getName(bool to_undo) const {
	return _ACTION_("ungroup parts");
}
