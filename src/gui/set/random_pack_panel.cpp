//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/random_pack_panel.hpp>
#include <gui/control/card_viewer.hpp>
#include <gui/control/filtered_card_list.hpp>
#include <data/game.hpp>
#include <data/pack.hpp>
#include <wx/spinctrl.h>

DECLARE_TYPEOF_COLLECTION(PackTypeP);
DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : RandomCardList

/// A card list that contains the 
class RandomCardList : public CardListBase {
  public:
	RandomCardList(Window* parent, int id, long style = 0);
	
	/// Reset the list
	void reset();
	/// Add a pack of cards
	void add(PackType& pack);
	
  protected:
	virtual void getItems(vector<VoidP>& out) const;
	virtual void onChangeSet();
	
  private:	
	vector<CardP> cards;
};

RandomCardList::RandomCardList(Window* parent, int id, long style)
	: CardListBase(parent, id, style)
{}

void RandomCardList::reset() {
	cards.clear();
}
void RandomCardList::add(PackType& pack) {
	pack.generate(*set,cards);
}

void RandomCardList::onChangeSet() {
	reset();
	CardListBase::onChangeSet();
}

void RandomCardList::getItems(vector<VoidP>& out) const {
	out.reserve(cards.size());
	FOR_EACH_CONST(c, cards) {
		out.push_back(c);
	}
}


// ----------------------------------------------------------------------------- : RandomPackPanel

RandomPackPanel::RandomPackPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	preview   = new CardViewer(this, wxID_ANY);
	card_list = new FilteredCardList(this, wxID_ANY);
	wxButton* generate = new wxButton(this, wxID_ANY, _BUTTON_("generate pack"));
	wxRadioButton* seed_random = new wxRadioButton(this, wxID_ANY, _BUTTON_("random seed"));
	wxRadioButton* seed_fixed  = new wxRadioButton(this, wxID_ANY, _BUTTON_("fixed seed"));
	seed = new wxTextCtrl(this, wxID_ANY);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(preview, 0, wxRIGHT,  2);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				wxSizer* s4 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack selection"));
					packsSizer = new wxFlexGridSizer(0, 2, 4, 8);
					packsSizer->AddGrowableCol(0);
					s4->AddSpacer(2);
					s4->Add(packsSizer, 1, wxEXPAND | wxALL & ~wxTOP, 4);
				s3->Add(s4, 1, wxEXPAND, 8);
				wxSizer* s5 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack totals"));
				s3->Add(s5, 1, wxEXPAND | wxLEFT, 8);
				wxSizer* s6 = new wxBoxSizer(wxVERTICAL);
					wxSizer* s7 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("seed"));
						s7->Add(seed_random, 0, wxALL, 4);
						s7->Add(seed_fixed,  0, wxALL, 4);
						wxSizer* s8 = new wxBoxSizer(wxHORIZONTAL);
							s8->Add(seed, 1, wxLEFT, 20);
						s7->Add(s8,          0, wxALL & ~wxTOP, 4);
					s6->Add(s7,       0, 0, 8);
					s6->AddStretchSpacer();
					s6->Add(generate, 0, wxTOP | wxALIGN_RIGHT, 8);
				s3->Add(s6, 0, wxEXPAND | wxLEFT, 8);
			s2->Add(s3, 0, wxEXPAND | wxALL & ~wxTOP, 4);
			s2->Add(card_list, 1, wxEXPAND);
		s->Add(s2,      1, wxEXPAND, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void RandomPackPanel::onChangeSet() {
	preview  ->setSet(set);
	card_list->setSet(set);
	
	// TODO: remove or reuse old pack controls if there are any.
	// add pack controls
	FOR_EACH(pack, set->game->pack_types) {
		PackItem i;
		i.pack  = pack;
		i.label = new wxStaticText(this, wxID_ANY, pack->name);
		i.value = new wxSpinCtrl(this, wxID_ANY, _("0"), wxDefaultPosition, wxSize(50,-1));
		packsSizer->Add(i.label, 0, wxALIGN_CENTER_VERTICAL);
		packsSizer->Add(i.value, 0, wxEXPAND | wxALIGN_CENTER);
		packs.push_back(i);
	}
	
	Layout();
}

// ----------------------------------------------------------------------------- : UI

void RandomPackPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// ?
}

void RandomPackPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	// ?
}

void RandomPackPanel::onUpdateUI(wxUpdateUIEvent& ev) {
	// ?
}

void RandomPackPanel::onCommand(int id) {
	// ?
}

// ----------------------------------------------------------------------------- : Generating

void RandomPackPanel::generate() {
	//set->game->pack_types[0].generate()
}

// ----------------------------------------------------------------------------- : Clipboard

bool RandomPackPanel::canCopy()  const { return card_list->canCopy();  }
void RandomPackPanel::doCopy()         {        card_list->doCopy();   }
