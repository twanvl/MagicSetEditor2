//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/card_select_window.hpp>
#include <gui/control/select_card_list.hpp>
#include <util/window_id.hpp>
#include <wx/statline.h>

DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(ExportCardSelectionChoiceP);

// ----------------------------------------------------------------------------- : ExportCardSelectionChoice

ExportCardSelectionChoice::ExportCardSelectionChoice()
	: label(_BUTTON_("export custom cards selection"))
	, type(EXPORT_SEL_CUSTOM)
	, the_cards(&own_cards)
{}
ExportCardSelectionChoice::ExportCardSelectionChoice(const Set& set)
	: label(_BUTTON_("export entire set"))
	, type(EXPORT_SEL_ENTIRE_SET)
	, the_cards(&set.cards)
{}
ExportCardSelectionChoice::ExportCardSelectionChoice(const String& label, const vector<CardP>& cards)
	: label(label)
	, type(EXPORT_SEL_SUBSET)
	, the_cards(&own_cards)
	, own_cards(cards)
{}
ExportCardSelectionChoice::ExportCardSelectionChoice(const String& label, const vector<CardP>* cards)
	: label(label)
	, type(EXPORT_SEL_SUBSET)
	, the_cards(cards)
{}

// ----------------------------------------------------------------------------- : ExportWindowBase


ExportWindowBase::ExportWindowBase(Window* parent, const String& title, const SetP& set, const ExportCardSelectionChoices& cards_choices, long style)
	: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, style)
	, set(set), cards_choices(cards_choices)
	, active_choice(0)
	, select_cards(nullptr)
{}

wxSizer* ExportWindowBase::Create() {
	// create sizer
	wxSizer* s = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("select cards"));
	// create choice radio buttons
	int i = 0;
	bool any_custom = false;
	FOR_EACH(choice, cards_choices) {
		wxRadioButton* btn = new wxRadioButton(this, ID_SELECTION_CHOICE + i, choice->label);
		btn->SetValue(i == 0);
		btn->Enable(!choice->the_cards->empty() || choice->type == EXPORT_SEL_CUSTOM);
		s->Add(btn, 0, wxALL, 6);
		s->AddSpacer(-4);
		any_custom |= choice->type == EXPORT_SEL_CUSTOM;
		i++;
	}
	// custom selection button
	if (any_custom) {
		select_cards = new wxButton(this, ID_SELECT_CARDS, _BUTTON_("select cards"));
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			s2->Add(select_cards, 1, wxLEFT, 20);
		s->AddSpacer(4);
		s->Add(s2, 0, wxALL & ~wxTOP, 6);
	}
	// total count label
	s->AddSpacer(4);
	s->Add(new wxStaticLine(this), 0, wxALL | wxEXPAND, 4);
	s->AddSpacer(4);
	card_count = new wxStaticText(this, wxID_ANY, wxEmptyString);
	s->Add(card_count, 0, wxALL & ~wxTOP, 6);
	s->AddSpacer(4);
	// done
	update();
	return s;
}

void ExportWindowBase::onChangeSelectionChoice(wxCommandEvent& ev) {
	active_choice = ev.GetId() - ID_SELECTION_CHOICE;
	update();
}

void ExportWindowBase::onSelectCards(wxCommandEvent&) {
	CardSelectWindow wnd(this, set, _LABEL_("select cards"), _TITLE_("select cards"));
	ExportCardSelectionChoice& choice = *cards_choices.at(active_choice);
	wnd.setSelection(choice.own_cards);
	if (wnd.ShowModal() != wxID_OK) {
		return; // cancel
	}
	// store cards
	choice.own_cards.clear();
	wnd.getSelection(choice.own_cards);
	update();
	
}

void ExportWindowBase::update() {
	ExportCardSelectionChoice& choice = *cards_choices.at(active_choice);
	cards = choice.the_cards;
	if (select_cards) {
		select_cards->Enable(choice.type == EXPORT_SEL_CUSTOM);
	}
	card_count->SetLabel(_LABEL_1_("selected card count", String::Format(_("%d"),cards->size())));
	wxWindow* ok_btn = FindWindow(wxID_OK);
	if (ok_btn) ok_btn->Enable(!cards->empty());
}


BEGIN_EVENT_TABLE(ExportWindowBase, wxDialog)
	EVT_RADIOBUTTON(wxID_ANY, ExportWindowBase::onChangeSelectionChoice)
	EVT_BUTTON     (ID_SELECT_CARDS,  ExportWindowBase::onSelectCards)
END_EVENT_TABLE  ()

// ----------------------------------------------------------------------------- : CardSelectWindow

CardSelectWindow::CardSelectWindow(Window* parent, const SetP& set, const String& label, const String& title, bool sizer)
	: wxDialog(parent, wxID_ANY, _TITLE_("select cards"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, set(set)
{
	// init controls
	list = new SelectCardList(this, wxID_ANY);
	list->setSet(set);
	sel_all  = new wxButton(this, ID_SELECT_ALL,  _BUTTON_("select all"));
	sel_none = new wxButton(this, ID_SELECT_NONE, _BUTTON_("select none"));
	// init sizers
	if (sizer) {
		wxSizer* s = new wxBoxSizer(wxVERTICAL);
			if (!label.empty()) {
				s->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALL & ~wxBOTTOM, 8);
			}
			s->Add(list, 1, wxEXPAND | wxALL, 8);
			wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
				s2->Add(sel_all,  0, wxEXPAND | wxRIGHT, 8);
				s2->Add(sel_none, 0, wxEXPAND | wxRIGHT, 8);
				s2->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxEXPAND, 8);
			s->Add(s2, 0, wxEXPAND | (wxALL & ~wxTOP), 8);
		s->SetSizeHints(this);
		SetSizer(s);
		SetSize(600,500);
	}
}

bool CardSelectWindow::isSelected(const CardP& card) const {
	return list->isSelected(card);
}

void CardSelectWindow::getSelection(vector<CardP>& out) const {
	list->getSelection(out);
}

void CardSelectWindow::setSelection(const vector<CardP>& cards) {
	list->setSelection(cards);
}

void CardSelectWindow::onSelectAll(wxCommandEvent&) {
	list->selectAll();
}
void CardSelectWindow::onSelectNone(wxCommandEvent&) {
	list->selectNone();
}

BEGIN_EVENT_TABLE(CardSelectWindow, wxDialog)
	EVT_BUTTON       (ID_SELECT_ALL,  CardSelectWindow::onSelectAll)
	EVT_BUTTON       (ID_SELECT_NONE, CardSelectWindow::onSelectNone)
END_EVENT_TABLE  ()
