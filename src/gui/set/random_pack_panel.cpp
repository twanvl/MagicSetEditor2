//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/random_pack_panel.hpp>
#include <gui/set/window.hpp>
#include <gui/control/card_viewer.hpp>
#include <gui/control/filtered_card_list.hpp>
#include <data/game.hpp>
#include <data/pack.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <wx/spinctrl.h>
#include <boost/random/mersenne_twister.hpp>

DECLARE_TYPEOF_COLLECTION(PackTypeP);
DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(RandomPackPanel::PackItem_for_typeof);

// ----------------------------------------------------------------------------- : RandomCardList

/// A card list that contains the 
class RandomCardList : public CardListBase {
  public:
	RandomCardList(Window* parent, int id, long style = 0);
	
	/// Reset the list
	void reset();
	/// Add a pack of cards
	void add(PackItemCache& packs, boost::mt19937& gen, const PackType& pack);
	
	using CardListBase::rebuild;
	
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
void RandomCardList::add(PackItemCache& packs, boost::mt19937& gen, const PackType& pack) {
	pack.generate(packs,gen,cards);
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
	card_list = new RandomCardList(this, wxID_ANY);
	generate_button = new wxButton(this, ID_GENERATE_PACK, _BUTTON_("generate pack"));
	seed_random = new wxRadioButton(this, ID_SEED_RANDOM, _BUTTON_("random seed"));
	seed_fixed  = new wxRadioButton(this, ID_SEED_FIXED,  _BUTTON_("fixed seed"));
	seed = new wxTextCtrl(this, wxID_ANY);
	static_cast<SetWindow*>(parent)->setControlStatusText(seed_random, _HELP_("random seed"));
	static_cast<SetWindow*>(parent)->setControlStatusText(seed_fixed,  _HELP_("fixed seed"));
	static_cast<SetWindow*>(parent)->setControlStatusText(seed,        _HELP_("seed"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(preview, 0, wxRIGHT,  2);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				wxSizer* s4 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack selection"));
					packsSizer = new wxFlexGridSizer(0, 2, 4, 8);
					packsSizer->AddGrowableCol(0);
					//s4->AddSpacer(2);
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
					//s6->AddStretchSpacer();
					//s6->Add(generate_button, 0, wxTOP | wxALIGN_RIGHT, 8);
					s6->Add(generate_button, 1, wxTOP | wxEXPAND, 8);
				s3->Add(s6, 0, wxEXPAND | wxLEFT, 8);
			s2->Add(s3, 0, wxEXPAND | wxALL & ~wxTOP, 4);
			s2->Add(card_list, 1, wxEXPAND);
		s->Add(s2,      1, wxEXPAND, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

RandomPackPanel::~RandomPackPanel() {
	storeSettings();
}

void RandomPackPanel::onChangeSet() {
	storeSettings();
	preview  ->setSet(set);
	card_list->setSet(set);
	
	// remove old pack controls
	FOR_EACH(i, packs) {
		packsSizer->Detach(i.label);
		packsSizer->Detach(i.value);
		delete i.label;
		delete i.value;
	}
	packs.clear();
	
	// add pack controls
	FOR_EACH(pack, set->game->pack_types) {
		PackItem i;
		i.pack  = pack;
		i.label = new wxStaticText(this, wxID_ANY, capitalize_sentence(pack->name));
		i.value = new wxSpinCtrl(this, ID_PACK_AMOUNT, _("0"), wxDefaultPosition, wxSize(50,-1));
		packsSizer->Add(i.label, 0, wxALIGN_CENTER_VERTICAL);
		packsSizer->Add(i.value, 0, wxEXPAND | wxALIGN_CENTER);
		packs.push_back(i);
	}
	
	Layout();
	
	// settings
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	seed_random->SetValue(gs.pack_seed_random);
	seed_fixed ->SetValue(!gs.pack_seed_random);
	seed->Enable(!gs.pack_seed_random);
	setSeed(gs.pack_seed);
	FOR_EACH(i, packs) {
		i.value->SetValue(gs.pack_amounts[i.pack->name]);
	}
	
	updateTotals();
}

void RandomPackPanel::storeSettings() {
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	gs.pack_seed_random = seed_random->GetValue();
	FOR_EACH(i, packs) {
		gs.pack_amounts[i.pack->name] = i.value->GetValue();
	}
}

// ----------------------------------------------------------------------------- : UI

void RandomPackPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {}

void RandomPackPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {}

void RandomPackPanel::onUpdateUI(wxUpdateUIEvent& ev) {}

void RandomPackPanel::onCommand(int id) {
	switch (id) {
		case ID_PACK_AMOUNT: {
			updateTotals();
			break;
		}
		case ID_GENERATE_PACK: {
			generate();
			break;
		}
		case ID_SEED_RANDOM: case ID_SEED_FIXED: {
			seed->Enable(seed_fixed->GetValue());
			break;
		}
	}
}

// ----------------------------------------------------------------------------- : Generating

void RandomPackPanel::updateTotals() {
	total_packs = 0;
	FOR_EACH(i,packs) {
		total_packs += i.value->GetValue();
	}
	// update UI
	generate_button->Enable(total_packs > 0);
}

int RandomPackPanel::getSeed() {
	// determine seed value
	int seed = 0;
	if (seed_random->GetValue()) {
		// use the C rand() function to get a seed
		seed = rand() % 1000 * 1000
		     + clock() % 1000;
	} else {
		// convert *any* string to a number
		String s = this->seed->GetValue();
		FOR_EACH_CONST(c,s) {
			seed *= 10;
			seed += abs(c - '0') + 123456789*(abs(c - '0')/10);
		}
	}
	setSeed(seed);
	return seed;
}
void RandomPackPanel::setSeed(int seed) {
	seed %= 1000000;
	this->seed->SetValue(wxString::Format(_("%06d"),seed));
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	gs.pack_seed = seed;
}

void RandomPackPanel::generate() {
	boost::mt19937 gen((unsigned)getSeed());
	PackItemCache pack_cache(*set);
	// add packs to card list
	card_list->reset();
	FOR_EACH(item,packs) {
		int copies = item.value->GetValue();
		for (int i = 0 ; i < copies ; ++i) {
			card_list->add(pack_cache, gen, *item.pack);
		}
	}
	card_list->rebuild();
}

// ----------------------------------------------------------------------------- : Selection

void RandomPackPanel::selectCard(const CardP& card) {
	preview->setCard(card);
}

// ----------------------------------------------------------------------------- : Clipboard

bool RandomPackPanel::canCopy()  const { return card_list->canCopy();  }
void RandomPackPanel::doCopy()         {        card_list->doCopy();   }
