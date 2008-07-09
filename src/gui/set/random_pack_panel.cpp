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

// ----------------------------------------------------------------------------- : RandomPackPanel

RandomPackPanel::RandomPackPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	preview   = new CardViewer(this, wxID_ANY);
	card_list = new FilteredCardList(this, wxID_ANY);
	wxButton* generate = new wxButton(this, wxID_ANY, _BUTTON_("generate pack"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(preview, 0, wxRIGHT,  2);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				wxSizer* s4 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack selection"));
					packsSizer = new wxFlexGridSizer(0, 2, 4, 8);
					packsSizer->AddGrowableCol(0);
					s4->Add(packsSizer, 1, wxEXPAND | wxALL, 4);
				s3->Add(s4, 1, wxEXPAND, 8);
				wxSizer* s5 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack totals"));
				s3->Add(s5, 1, wxEXPAND | wxLEFT, 8);
				s3->Add(generate, 0, wxALIGN_BOTTOM | wxLEFT, 8);
			s2->Add(s3, 0, wxEXPAND | wxALL & ~wxTOP, 4);
			s2->Add(card_list, 1, wxEXPAND);
		s->Add(s2,      1, wxEXPAND, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void RandomPackPanel::onChangeSet() {
	preview  ->setSet(set);
	card_list->setSet(set);
	
	// add pack controls
	FOR_EACH(pack, set->game->pack_types) {
		PackItem i;
		i.pack  = pack;
		i.label = new wxStaticText(this, wxID_ANY, pack->name);
		i.value = new wxSpinCtrl(this, wxID_ANY, _("0"), wxDefaultPosition, wxSize(50,-1));
		//i.value->SetSizeHints(0,0,50,100);
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
