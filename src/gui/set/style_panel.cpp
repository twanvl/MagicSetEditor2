//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/style_panel.hpp>
#include <gui/control/package_list.hpp>
#include <gui/control/card_viewer.hpp>
#include <gui/control/native_look_editor.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>
#include <data/action/set.hpp>
#include <data/action/value.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);

// ----------------------------------------------------------------------------- : StylePanel

StylePanel::StylePanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	preview       = new CardViewer   (this, wxID_ANY);
	list          = new PackageList  (this, wxID_ANY);
	use_for_all   = new wxButton     (this, ID_STYLE_USE_FOR_ALL, _BUTTON_("use for all cards"));
	use_custom_options = new wxCheckBox(this, ID_STYLE_USE_CUSTOM, _BUTTON_("use custom styling options"));
	editor        = new StylingEditor(this, ID_EDITOR, wxNO_BORDER);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(preview, 0, wxRIGHT,  2);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(list,        0, wxEXPAND | wxBOTTOM,                4);
			s2->Add(use_for_all, 0, wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, 4);
			wxSizer* s3 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("styling options"));
				s3->Add(use_custom_options, 0, wxEXPAND | wxALL, 4);
				s3->Add(editor,             2, wxEXPAND, 0);
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
	card = CardP();
	use_for_all->Enable(false);
}

void StylePanel::onAction(const Action& action, bool undone) {
	TYPE_CASE_(action, ChangeSetStyleAction) {
		list->select(set->stylesheetFor(card).name(), false);
		editor->showCard(card);
	}
	TYPE_CASE(action, ChangeCardStyleAction) {
		if (action.card == card) {
			list->select(set->stylesheetFor(card).name(), false);
			editor->showCard(card);
		}
	}
	TYPE_CASE(action, ChangeCardHasStylingAction) {
		if (action.card == card) {
			editor->showCard(card);
		}
	}
	TYPE_CASE(action, ValueAction) {
		// is it a styling action?
		if (!action.card) {
			const StyleSheet& s = set->stylesheetFor(card);
			FOR_EACH_CONST(f, s.styling_fields) {
				if (action.valueP->fieldP == f) {
					// refresh the viewer
					preview->redraw();
					return;
				}
			}
		}
	}
	use_for_all->Enable(card && card->stylesheet);
	use_custom_options->Enable(card);
	use_custom_options->SetValue(card ? card->has_styling : false);
}

// ----------------------------------------------------------------------------- : Selection

void StylePanel::selectCard(const CardP& card) {
	this->card = card;
	preview->setCard(card);
	editor->showStylesheet(set->stylesheetForP(card));
	editor->showCard(card);
	list->select(set->stylesheetFor(card).name(), false);
	use_for_all->Enable(card && card->stylesheet);
	use_custom_options->Enable(card);
	use_custom_options->SetValue(card ? card->has_styling : false);
}

// ----------------------------------------------------------------------------- : Clipboard

// determine what control to use for clipboard actions
#define CUT_COPY_PASTE(op,return)					\
	int id = focused_control(this);					\
	if (id == ID_EDITOR) { return editor->op(); }	\
	else                 { return false;          }

bool StylePanel::canCopy()  const { CUT_COPY_PASTE(canCopy,  return) }
bool StylePanel::canCut()   const { CUT_COPY_PASTE(canCut,   return) }
bool StylePanel::canPaste() const { CUT_COPY_PASTE(canPaste, return) }
void StylePanel::doCopy()         { CUT_COPY_PASTE(doCopy,   return (void)) }
void StylePanel::doCut()          { CUT_COPY_PASTE(doCut,    return (void)) }
void StylePanel::doPaste()        { CUT_COPY_PASTE(doPaste,  return (void)) }

// ----------------------------------------------------------------------------- : Events

void StylePanel::onStyleSelect(wxCommandEvent&) {
	if (list->hasSelection() && card) {
		StyleSheetP stylesheet = list->getSelection<StyleSheet>();
		if (stylesheet->game != set->game) {
			throw PackageError(_("Stylesheet made for the wrong game"));
		}
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

void StylePanel::onUseCustom(wxCommandEvent&) {
	set->actions.add(new ChangeCardHasStylingAction(*set, card));
}

BEGIN_EVENT_TABLE(StylePanel, wxPanel)
	EVT_GALLERY_SELECT(wxID_ANY,             StylePanel::onStyleSelect)
	EVT_BUTTON        (ID_STYLE_USE_FOR_ALL, StylePanel::onUseForAll)
	EVT_CHECKBOX      (ID_STYLE_USE_CUSTOM,  StylePanel::onUseCustom)
END_EVENT_TABLE()
