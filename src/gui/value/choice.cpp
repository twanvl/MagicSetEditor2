//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/choice.hpp>
#include <gui/util.hpp>
#include <data/action/value.hpp>

DECLARE_TYPEOF_COLLECTION(ChoiceField::ChoiceP);

// ----------------------------------------------------------------------------- : DropDownChoiceList

DropDownChoiceList::DropDownChoiceList(Window* parent, bool is_submenu, ChoiceValueEditor& cve, ChoiceField::ChoiceP group)
	: DropDownList(parent, is_submenu, is_submenu ? nullptr : &cve)
	, group(group)
	, cve(cve)
{}

size_t DropDownChoiceList::itemCount() const {
	return group->choices.size() + hasDefault();
}

ChoiceField::ChoiceP DropDownChoiceList::getChoice(size_t item) const {
	if (isGroupDefault(item)) {
		return group;
	} else {
		return group->choices[item - hasDefault()];
	}
}

String DropDownChoiceList::itemText(size_t item) const {
	if (isFieldDefault(item)) {
		return field().default_name;
	} else {
		ChoiceField::ChoiceP choice = getChoice(item);
		return choice->name;
	}
}
bool DropDownChoiceList::lineBelow(size_t item) const {
	return isDefault(item);
}
DropDownList* DropDownChoiceList::submenu(size_t item) {
	if (isDefault(item)) return nullptr;
	item -= hasDefault();
	if (item < submenus.size()) submenus.resize(item + 1);
	if (submenus[item]) return submenus[item].get();
	ChoiceField::ChoiceP choice = getChoice(item);
	if (choice->isGroup()) {
		// create submenu
		submenus[item].reset(new DropDownChoiceList(GetParent(), true, cve, choice));
	}
	return submenus[item].get();
}

void DropDownChoiceList::drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {
	// TODO
}


void DropDownChoiceList::select(size_t item) {
	if (isFieldDefault(item)) {
		cve.change( Defaultable<String>() );
	} else {
		ChoiceField::ChoiceP choice = getChoice(item);
		cve.change( field().choices->choiceName(choice->first_id) );
	}
}
size_t DropDownChoiceList::selection() const {
	if (hasFieldDefault() && cve.value().value.isDefault()) {
		return 0;
	}
	size_t i = hasDefault();
	int id = field().choices->choiceId(cve.value().value());
	FOR_EACH(c, group->choices) {
		if (id >= c->first_id && id < c->lastId()) {
			return i;
		}
		i++;
	}
	return NO_SELECTION;
}

// ----------------------------------------------------------------------------- : ChoiceValueEditor

IMPLEMENT_VALUE_EDITOR(Choice)
	, drop_down(new DropDownChoiceList(&editor(), false, *this, field().choices))
{}

void ChoiceValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	drop_down->onMouseInParent(ev, style().popup_style == POPUP_DROPDOWN_IN_PLACE && !nativeLook());
}
void ChoiceValueEditor::onChar(wxKeyEvent& ev) {
	drop_down->onCharInParent(ev);
}
void ChoiceValueEditor::onLoseFocus() {
	drop_down->hide(false);
}

void ChoiceValueEditor::drawSelection(RotatedDC& dc) {
	if (nativeLook()) {
		draw_drop_down_arrow(&editor(), dc.getDC(), style().getRect().grow(1), drop_down->IsShown());
	}
}
void ChoiceValueEditor::determineSize() {
	style().height = max(style().height(), 16.);
}

void ChoiceValueEditor::change(const Defaultable<String>& c) {
	getSet().actions.add(value_action(static_pointer_cast<ChoiceValue>(valueP), c));
}
