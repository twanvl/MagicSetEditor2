//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/style_panel.hpp>
#include <gui/control/package_list.hpp>
#include <gui/control/card_viewer.hpp>
#include <gui/control/native_look_editor.hpp>
#include <util/window_id.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>
#include <data/action/set.hpp>

// ----------------------------------------------------------------------------- : StylePanel

StylePanel::StylePanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	preview     = new CardViewer   (this, wxID_ANY);
	editor      = new StylingEditor(this, wxID_ANY, wxNO_BORDER);
	list        = new PackageList  (this, wxID_ANY);
	use_for_all = new wxButton     (this, ID_STYLE_USE_FOR_ALL, _("Use for &all cards"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(preview, 0, wxRIGHT,  2);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(list,        0, wxEXPAND | wxBOTTOM,                4);
			s2->Add(use_for_all, 0, wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, 4);
			wxSizer* s3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Extra styling options"));
				s3->Add(editor, 2, wxEXPAND, 0);
			s2->Add(s3, 1, wxEXPAND | wxALL, 2);
		s->Add(s2,      1, wxEXPAND, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void StylePanel::onChangeSet() {
	list->showData<StyleSheet>(set->game->name() + _("-*"));
	list->select(set->stylesheet->name(), false);
	editor->setSet(set);
	preview->setSet(set);
	card.reset();
	use_for_all->Enable(false);
}

void StylePanel::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, ChangeSetStyleAction) {
		list->select(set->stylesheetFor(card)->name(), false);
//		updateSize();
	}
	TYPE_CASE(action, ChangeCardStyleAction) {
		if (action.card == card) {
//			preview->onAction(action, undone); // update the preview control before we determine our new size
			list->select(set->stylesheetFor(card)->name(), false);
//			updateSize();
		}
	}
}

// ----------------------------------------------------------------------------- : Selection

void StylePanel::selectCard(const CardP& card) {
	this->card = card;
	preview->setCard(card);
	list->select(set->stylesheetFor(card)->name(), false);
	use_for_all->Enable(card && card->stylesheet);
}

// ----------------------------------------------------------------------------- : Events

void StylePanel::onStyleSelect(wxCommandEvent&) {
	if (list->hasSelection()) {
		StyleSheetP stylesheet = list->getSelection<StyleSheet>();
		if (stylesheet == set->stylesheet) {
			// select no special style when selecting the same style as the set default
			stylesheet = StyleSheetP();
		}
		set->actions.add(new ChangeCardStyleAction(card, stylesheet));
		Layout();
	}
}

void StylePanel::onUseForAll(wxCommandEvent&) {
	set->actions.add(new ChangeSetStyleAction(*set, card));
	Layout();
}

BEGIN_EVENT_TABLE(StylePanel, wxPanel)
	EVT_GALLERY_SELECT(wxID_ANY,             StylePanel::onStyleSelect)
	EVT_BUTTON        (ID_STYLE_USE_FOR_ALL, StylePanel::onUseForAll)
END_EVENT_TABLE()
