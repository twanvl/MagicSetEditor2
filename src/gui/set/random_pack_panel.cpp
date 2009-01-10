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
#include <gui/util.hpp>
#include <gui/about_window.hpp> // HoverButtonBase
#include <data/game.hpp>
#include <data/pack.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <wx/spinctrl.h>
#include <wx/dcbuffer.h>

DECLARE_TYPEOF_COLLECTION(PackTypeP);
DECLARE_TYPEOF_COLLECTION(PackItemP);
#if !USE_NEW_PACK_SYSTEM
	DECLARE_TYPEOF_COLLECTION(PackItemRefP);
#endif
DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF_COLLECTION(PackAmountPicker);

// ----------------------------------------------------------------------------- : RandomCardList

/// A card list that contains the 
class RandomCardList : public CardListBase {
  public:
	RandomCardList(Window* parent, int id, long style = 0);
	
	/// Reset the list
	void reset();
  #if !USE_NEW_PACK_SYSTEM
	/// Add a pack of cards
	void add(PackItemCache& packs, boost::mt19937& gen, const PackType& pack);
  #endif
	
	using CardListBase::rebuild;
	
	const vector<CardP>* getCardsPtr() const { return &cards; }
	
  protected:
	virtual void getItems(vector<VoidP>& out) const;
	virtual void onChangeSet();
	
#if USE_NEW_PACK_SYSTEM
  public:
#else
  private:
#endif
	vector<CardP> cards;
};

RandomCardList::RandomCardList(Window* parent, int id, long style)
	: CardListBase(parent, id, style)
{}

void RandomCardList::reset() {
	cards.clear();
}

#if !USE_NEW_PACK_SYSTEM
	void RandomCardList::add(PackItemCache& packs, boost::mt19937& gen, const PackType& pack) {
		pack.generate(packs,gen,cards);
	}
#endif

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


// ----------------------------------------------------------------------------- : PackTotalsPanel

class PackTotalsPanel : public wxPanel {
  public:
   #if USE_NEW_PACK_SYSTEM
	PackTotalsPanel(Window* parent, int id, PackGenerator& generator, bool show_all = false)
		: wxPanel(parent,id), generator(generator), show_all(show_all) {}
   #else
	PackTotalsPanel(Window* parent, int id) : wxPanel(parent,id) {}
   #endif
	void setGame(const GameP& game);
   #if !USE_NEW_PACK_SYSTEM
	void clear();
	void addPack(PackType& pack, int copies);
	void addItemRef(PackItemRef& item, int copies);
   #endif
	virtual wxSize DoGetBestSize() const;
  private:
	DECLARE_EVENT_TABLE();
	GameP game;
	void onPaint(wxPaintEvent&);
	void draw(DC& dc);
	void drawItem(DC& dc, int& y, const String& name, double value);
  #if USE_NEW_PACK_SYSTEM
	PackGenerator& generator;
	bool show_all;
  #else
	map<String,int> amounts;
  #endif
};

void PackTotalsPanel::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	draw(dc);
}
void PackTotalsPanel::draw(DC& dc) {
	// clear background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	wxSize size = dc.GetSize();
	dc.DrawRectangle(0,0,size.x,size.y);
	// draw table
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	dc.SetFont(*wxNORMAL_FONT);
	int y = 0;
	int total = 0;
  #if USE_NEW_PACK_SYSTEM
	FOR_EACH(pack, game->pack_types) {
		PackInstance& i = generator.get(pack);
		if (pack->summary && (show_all || i.has_cards())) {
			drawItem(dc, y, tr(*game, pack->name, capitalize), i.get_card_copies());
			total += (int)i.get_card_copies();
		}
	}
  #else
	FOR_EACH(item, game->pack_items) {
		int value = amounts[item->name];
		drawItem(dc, y, tr(*game, item->name, capitalize), value);
		total += value;
	}
  #endif
	// draw total
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
	dc.DrawLine(0, y-3, size.x, y-3);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT));
	dc.DrawLine(0, y-2, size.x, y-2);
	y += 7;
	drawItem(dc, y, _LABEL_("total cards"), total);
}
void PackTotalsPanel::drawItem(DC& dc, int& y,  const String& name, double value) {
	wxSize size = dc.GetSize();
	int w,h;
	String amount = String::Format(_("%.f"),value);
	dc.GetTextExtent(amount,&w,&h);
	dc.DrawText(name,   0,        y);
	dc.DrawText(amount, size.x-w, y);//align right
	y += h + 10;
}

wxSize PackTotalsPanel::DoGetBestSize() const {
	// count lines
	int lines = 0;
  #if USE_NEW_PACK_SYSTEM
	if (game && generator.set) {
		FOR_EACH(pack, game->pack_types) {
			PackInstance& i = generator.get(pack);
			if (pack->summary && (show_all || i.has_cards())) {
				lines++;
			}
		}
	}
  #else
	lines = game ? (int)game->pack_items.size() : 0;
  #endif
	// don't forget the total
	lines++;
	// size
	int height = lines * (GetCharHeight() + 10) + 7 - 10;
	wxSize ws = GetSize(), cs = GetClientSize();
	return wxSize(0,height) + ws - cs;
}

void PackTotalsPanel::setGame(const GameP& game) {
	this->game = game;
  #if !USE_NEW_PACK_SYSTEM
	clear();
  #endif
}
#if !USE_NEW_PACK_SYSTEM
	void PackTotalsPanel::clear() {
		amounts.clear();
	}
	void PackTotalsPanel::addPack(PackType& pack, int copies) {
		FOR_EACH(item,pack.items) {
			addItemRef(*item, copies * item->amount);
		}
	}
	void PackTotalsPanel::addItemRef(PackItemRef& item, int copies) {
		amounts[item.name] += copies;
	}
#endif

BEGIN_EVENT_TABLE(PackTotalsPanel, wxPanel)
	EVT_PAINT(PackTotalsPanel::onPaint)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------- : SelectableLabel

class SelectableLabel : public HoverButtonBase {
  public:
	SelectableLabel(wxWindow* parent, int id, const String& label, bool interactive = true)
		: HoverButtonBase(parent, id, false)
		, label(label)
		, interactive(interactive)
		, buddy(nullptr)
	{}
	void draw(DC& dc) {
		Color bg = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
		Color fg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
		// clear background
		dc.SetPen(*wxTRANSPARENT_PEN);
		//dc.SetBrush(mouse_down ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)
		//                       : hover ? lerp(bg,fg,0.1) : bg);
		//dc.SetTextForeground(mouse_down ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : fg);
		dc.SetBrush(interactive && hover ? lerp(bg,fg,0.1) : bg);
		dc.SetTextForeground(fg);
		wxSize size = dc.GetSize();
		dc.DrawRectangle(0,0,size.x,size.y);
		// draw label
		dc.SetFont(*wxNORMAL_FONT);
		int w,h;
		wxSize s = dc.GetSize();
		dc.GetTextExtent(label,&w,&h);
		dc.DrawText(interactive && hover ? label + _("...") : label, 2, (s.y-h)/2);
	}
	wxSize DoGetBestSize() const {
		int w,h;
		wxClientDC dc(const_cast<SelectableLabel*>(this));
		dc.SetFont(*wxNORMAL_FONT);
		dc.GetTextExtent(label,&w,&h);
		return wxSize(w+6,h);
	}
	void setBuddy(wxWindow* buddy) {
		this->buddy = buddy;
	}
	virtual void onClick() {
		if (buddy) buddy->SetFocus();
	}
	void onDoubleClick(wxMouseEvent&) {
		if (interactive) HoverButtonBase::onClick();
	}
  private:
	String label;
	bool interactive;
	wxWindow* buddy;
	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(SelectableLabel, HoverButtonBase)
	EVT_LEFT_DCLICK(SelectableLabel::onDoubleClick)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : PackAmountPicker

PackAmountPicker::PackAmountPicker(wxWindow* parent, wxFlexGridSizer* sizer, const PackTypeP& pack, bool active)
	: pack(pack)
	, label(new SelectableLabel(parent, ID_PACK_TYPE, capitalize_sentence(pack->name), active))
	, value(new wxSpinCtrl(parent, ID_PACK_AMOUNT, _("0"), wxDefaultPosition, wxSize(50,-1)))
{
	label->setBuddy(value);
	sizer->Add(label, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
	sizer->Add(value, 0, wxEXPAND | wxALIGN_CENTER);
	if (active) {
		label->SetHelpText(_("Double click to edit."));
	}
	set_help_text(value, _("The number of ") + pack->name + _("s to use."));
}

void PackAmountPicker::destroy(wxFlexGridSizer* sizer) {
	sizer->Detach(label);
	sizer->Detach(value);
	delete label;
	delete value;
}

// ----------------------------------------------------------------------------- : CustomPackDialog
#if USE_NEW_PACK_SYSTEM

class CustomPackDialog : public wxDialog {
  public:
	CustomPackDialog(Window* parent, const SetP& set, const PackTypeP& edited_pack);
	PackTypeP get() const { return edited_pack; }
  private:
	DECLARE_EVENT_TABLE();
	
	SetP             set;
	PackTypeP        edited_pack;
	PackGenerator    generator;
	wxTextCtrl*      name;
	PackTotalsPanel* totals;
	vector<PackAmountPicker> pickers;
	
	void updateTotals();
	void storePack();
	void onAmountChange(wxSpinEvent&);
	void onOk(wxCommandEvent&);
};

CustomPackDialog::CustomPackDialog(Window* parent, const SetP& set, const PackTypeP& edited_pack)
	: wxDialog(parent, wxID_ANY, _TITLE_("custom pack"), wxDefaultPosition, wxSize(500,500))
	, set(set), edited_pack(edited_pack)
{
	// init ui
	totals = new PackTotalsPanel(this, wxID_ANY, generator, true);
	name   = new wxTextCtrl(this, wxID_ANY, edited_pack ? edited_pack->name : _("custom pack"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack name"));
			s2->Add(name, 1, wxEXPAND | wxALL, 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
		wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
			wxSizer* s4 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack selection"));
				s4->AddSpacer(2);
				wxFlexGridSizer* packsSizer = new wxFlexGridSizer(0, 2, 4, 4);
				packsSizer->AddGrowableCol(0);
				s4->Add(packsSizer, 1, wxEXPAND | wxALL & ~wxTOP & ~wxLEFT, 4);
			s3->Add(s4, 1, wxEXPAND, 8);
			wxSizer* s5 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack totals"));
				s5->Add(totals, 1, wxEXPAND | wxALL, 4);
			s3->Add(s5, 1, wxEXPAND | wxLEFT, 8);
		s->Add(s3, 0, wxEXPAND | wxALL & ~wxTOP, 8);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP, 8);
	// add spin controls
	FOR_EACH(pack, set->game->pack_types) {
		if (pack->selectable) continue; // this pack is already selectable from the main UI
		PackAmountPicker pick(this, packsSizer, pack, false);
		pickers.push_back(pick);
		// set value if it is nonzero
		if (edited_pack) {
			FOR_EACH(i, edited_pack->items) {
				if (i->name == pack->name) {
					pick.value->SetValue(i->amount);
				}
			}
		}
	}
	s->SetSizeHints(this);
	SetSizer(s);
	// update totals
	generator.reset(set,0);
	totals->setGame(set->game);
	updateTotals();
}

void CustomPackDialog::updateTotals() {
	generator.gen.seed(0);
	int total_packs = 0;
	FOR_EACH(pick,pickers) {
		int copies = pick.value->GetValue();
		total_packs += copies;
		generator.get(pick.pack).request_copy(copies);
	}
	generator.update_card_counts();
	// update UI
	totals->Refresh(false);
	FindWindow(wxID_OK)->Enable(total_packs > 0);
}

void CustomPackDialog::storePack() {
	edited_pack = new_intrusive<PackType>();
	edited_pack->selectable = true;
	edited_pack->select = SELECT_ALL;
	edited_pack->name = name->GetValue();
	FOR_EACH(pick,pickers) {
		int copies = pick.value->GetValue();
		if (copies > 0) {
			edited_pack->items.push_back(new_intrusive2<PackItem>(pick.pack->name, copies));
		}
	}
}

void CustomPackDialog::onOk(wxCommandEvent&) {
	storePack();
	EndModal(wxID_OK);
}
void CustomPackDialog::onAmountChange(wxSpinEvent&) {
	updateTotals();
}

BEGIN_EVENT_TABLE(CustomPackDialog, wxDialog)
	EVT_BUTTON   (wxID_OK,        CustomPackDialog::onOk)
	EVT_SPINCTRL (ID_PACK_AMOUNT, CustomPackDialog::onAmountChange)
END_EVENT_TABLE()

#endif
// ----------------------------------------------------------------------------- : RandomPackPanel

RandomPackPanel::RandomPackPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// delayed initialization by initControls()
}

void RandomPackPanel::initControls() {
	// init controls
	preview   = new CardViewer(this, wxID_ANY);
	card_list = new RandomCardList(this, wxID_ANY);
	generate_button = new wxButton(this, ID_GENERATE_PACK, _BUTTON_("generate pack"));
	seed_random = new wxRadioButton(this, ID_SEED_RANDOM, _BUTTON_("random seed"));
	seed_fixed  = new wxRadioButton(this, ID_SEED_FIXED,  _BUTTON_("fixed seed"));
	seed = new wxTextCtrl(this, wxID_ANY);
  #if USE_NEW_PACK_SYSTEM
	totals = new PackTotalsPanel(this, wxID_ANY, generator);
  #else
	totals = new PackTotalsPanel(this, wxID_ANY);
  #endif
	set_help_text(seed_random, _HELP_("random seed"));
	set_help_text(seed_fixed,  _HELP_("fixed seed"));
	set_help_text(seed,        _HELP_("seed"));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(preview, 0, wxRIGHT,  2);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				wxSizer* s4 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("pack selection"));
					wxSizer* s4b = new wxBoxSizer(wxHORIZONTAL);
						packsSizer = new wxFlexGridSizer(0, 2, 4, 4);
						packsSizer->AddGrowableCol(0);
						s4b->Add(packsSizer, 1, wxEXPAND | wxALL & ~wxTOP & ~wxBOTTOM & ~wxLEFT, 4);
					s4->Add(s4b, 1, wxEXPAND | wxLEFT, 2);
					s4->Add(new wxButton(this, ID_CUSTOM_PACK, _BUTTON_("custom pack")), 0, wxEXPAND | wxALL & ~wxTOP, 4);
				s3->Add(s4, 1, wxEXPAND, 8);
				wxSizer* s5 = new wxStaticBoxSizer(wxHORIZONTAL, this, _LABEL_("pack totals"));
					s5->Add(totals, 1, wxEXPAND | wxALL, 4);
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

void RandomPackPanel::onBeforeChangeSet() {
	if (set) {
		storeSettings();
	}
}
void RandomPackPanel::onChangeSet() {
	if (!isInitialized()) return;
	preview  ->setSet(set);
	card_list->setSet(set);
	totals   ->setGame(set->game);
	
	// remove old pack controls
	FOR_EACH(pick, pickers) pick.destroy(packsSizer);
	pickers.clear();
	
	// add pack controls
	FOR_EACH(pack, set->game->pack_types) {
	  #if USE_NEW_PACK_SYSTEM
		if (pack->selectable) {
	  #endif
			pickers.push_back(PackAmountPicker(this,packsSizer,pack));
	  #if USE_NEW_PACK_SYSTEM
		}
	  #endif
	}
	
	Layout();
	
	// settings
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	seed_random->SetValue(gs.pack_seed_random);
	seed_fixed ->SetValue(!gs.pack_seed_random);
	seed->Enable(!gs.pack_seed_random);
	setSeed(gs.pack_seed);
	FOR_EACH(pick, pickers) {
		pick.value->SetValue(gs.pack_amounts[pick.pack->name]);
	}
	
  #if USE_NEW_PACK_SYSTEM
	generator.reset(set,last_seed=getSeed());
  #endif
	updateTotals();
}

void RandomPackPanel::storeSettings() {
	if (!isInitialized()) return;
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	gs.pack_seed_random = seed_random->GetValue();
	FOR_EACH(pick, pickers) {
		gs.pack_amounts[pick.pack->name] = pick.value->GetValue();
	}
}

// ----------------------------------------------------------------------------- : UI

void RandomPackPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// Init controls?
	if (!isInitialized()) {
		wxBusyCursor busy;
		initControls();
		onChangeSet();
	}
	// this is a good moment to update, because the set has likely changed
  #if USE_NEW_PACK_SYSTEM
	generator.reset(set,last_seed);
	updateTotals();
  #endif
}

void RandomPackPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {}

void RandomPackPanel::onUpdateUI(wxUpdateUIEvent& ev) {}

void RandomPackPanel::onCommand(int id) {
	if (!isInitialized()) return;
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
		case ID_CUSTOM_PACK: {
			CustomPackDialog dlg(this, set, PackTypeP());
			if (dlg.ShowModal() == wxID_OK) {
				// TODO: add pack
			}
			break;
		}
	}
}
void RandomPackPanel::onPackTypeClick(wxCommandEvent& ev) {
	FOR_EACH(pick,pickers) {
		if (pick.label == ev.GetEventObject()) {
			// edit this pack type
			CustomPackDialog dlg(this, set, pick.pack);
			if (dlg.ShowModal() == wxID_OK) {
				// TODO: update pack
			}
			break;
		}
	}
}

// ----------------------------------------------------------------------------- : Generating

void RandomPackPanel::updateTotals() {
  #if USE_NEW_PACK_SYSTEM
	generator.gen.seed((unsigned)last_seed);
  #else
	totals->clear();
  #endif
	int total_packs = 0;
	FOR_EACH(pick,pickers) {
		int copies = pick.value->GetValue();
		total_packs += copies;
	  #if USE_NEW_PACK_SYSTEM
		generator.get(pick.pack).request_copy(copies);
	  #else
		totals->addPack(*pick.pack, copies);
	  #endif
	}
  #if USE_NEW_PACK_SYSTEM
	generator.update_card_counts();
  #endif
	// update UI
	totals->Refresh(false);
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
  #if USE_NEW_PACK_SYSTEM
	generator.reset(set,last_seed=getSeed());
  #else
	boost::mt19937 gen((unsigned)getSeed());
	PackItemCache pack_cache(*set);
  #endif
	// add packs to card list
	card_list->reset();
	FOR_EACH(pick,pickers) {
		int copies = pick.value->GetValue();
		for (int i = 0 ; i < copies ; ++i) {
		  #if USE_NEW_PACK_SYSTEM
			generator.get(pick.pack).request_copy();
			generator.generate(card_list->cards);
		  #else
			card_list->add(pack_cache, gen, *pick.pack);
		  #endif
		}
	}
	card_list->rebuild();
	card_list->selectFirst();
}

// ----------------------------------------------------------------------------- : Selection

CardP RandomPackPanel::selectedCard() const {
	if (!isInitialized()) return CardP();
	return card_list->getCard();
}

void RandomPackPanel::selectCard(const CardP& card) {
	// Don't change the card based on other panels
	//preview->setCard(card);
}
void RandomPackPanel::onCardSelect(CardSelectEvent& ev) {
	preview->setCard(ev.getCard());
	ev.Skip(); // but do change other panels' selection
}

void RandomPackPanel::selectionChoices(ExportCardSelectionChoices& out) {
	if (!isInitialized()) return;
	out.push_back(new_intrusive2<ExportCardSelectionChoice>(
			_BUTTON_("export generated packs"),
			card_list->getCardsPtr()
		));
}


BEGIN_EVENT_TABLE(RandomPackPanel, wxPanel)
	EVT_CARD_SELECT(wxID_ANY, RandomPackPanel::onCardSelect)
	EVT_BUTTON     (ID_PACK_TYPE, RandomPackPanel::onPackTypeClick)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Clipboard

bool RandomPackPanel::canCopy()  const { return isInitialized() && card_list->canCopy();  }
void RandomPackPanel::doCopy()         {        isInitialized() && card_list->doCopy();   }
