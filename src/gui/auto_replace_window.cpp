//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/auto_replace_window.hpp>
#include <gui/control/item_list.hpp>
#include <gui/util.hpp>
#include <data/settings.hpp>
#include <data/game.hpp>
#include <data/word_list.hpp>
#include <gfx/gfx.hpp>
#include <util/window_id.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(AutoReplaceP);

// ----------------------------------------------------------------------------- : AutoReplaceList

DECLARE_EVENT_TYPE(EVENT_ITEM_SELECT, <not used>)
DEFINE_EVENT_TYPE(EVENT_ITEM_SELECT);

class AutoReplaceList : public ItemList {
  public:
	AutoReplaceList(Window* parent, int id, const Game& game);

	/// The items
	vector<AutoReplaceP> items;
	/// Settings we are edditing
	const Game& game;
	GameSettings& gs;

	/// The current item
	inline AutoReplaceP getSelected() const { return static_pointer_cast<AutoReplace>(selected_item); }

	/// Add an item
	void addItem(const AutoReplaceP& item);
	/// Remove the selected item
	void removeSelected();

	/// Reset the list to the default
	void reset();

	using ItemList::refreshList;

  protected:
	/// Get all items
	virtual void getItems(vector<VoidP>& out) const;
	/// Return the AutoReplace at the given position in the sorted list
	inline AutoReplaceP getAR(long pos) const { return static_pointer_cast<AutoReplace>(getItem(pos)); }

	/// Send an 'item selected' event for the currently selected item (selected_item)
	virtual void sendEvent();
	/// Compare items
	virtual bool compareItems(void* a, void* b) const;
	virtual bool mustSort() const { return true; }

	/// Get the text of an item in a specific column
	/** Overrides a function from wxListCtrl */
	virtual String OnGetItemText (long pos, long col) const;
	/// Get the image of an item, by default no image is used
	/** Overrides a function from wxListCtrl */
	virtual int    OnGetItemImage(long pos) const;
	/// Get the color for an item
	virtual wxListItemAttr* OnGetItemAttr(long pos) const;

	mutable wxListItemAttr item_attr; // for OnGetItemAttr
};

AutoReplaceList::AutoReplaceList(Window* parent, int id, const Game& game)
	: ItemList(parent, id)
	, game(game)
	, gs(settings.gameSettingsFor(game))
{
	// clone items
	FOR_EACH_CONST(ar, gs.auto_replaces) {
		items.push_back(ar->clone());
	}
	// Add columns
	InsertColumn(0, _(""),                   wxLIST_FORMAT_LEFT,    0); // dummy, prevent the image from taking up space
	InsertColumn(1, _LABEL_("auto match"),   wxLIST_FORMAT_LEFT,  100);
	InsertColumn(2, _LABEL_("auto replace"), wxLIST_FORMAT_LEFT,  200);
	// grey for disabled items
	item_attr.SetTextColour(lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
	                             wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),0.5));
	// init list
	refreshList();
	sortBy(1,true);
}

void AutoReplaceList::addItem(const AutoReplaceP& item) {
	items.push_back(item);
	refreshList();
	selectItem(item, true, true);
}

void AutoReplaceList::removeSelected() {
	for (size_t i = 0 ; i < items.size() ; ++i) {
		if (items[i] == selected_item) {
			items.erase(items.begin() + i);
			// select next
			refreshList();
			selectItem(items.empty() ? VoidP() : static_pointer_cast<IntrusivePtrVirtualBase>(items[min(i, items.size())]),
			           true, true);
			return;
		}
	}
}

void AutoReplaceList::reset() {
	// reset to list from game
	items.clear();
	FOR_EACH_CONST(ar, game.auto_replaces) {
		items.push_back(ar->clone());
	}
	refreshList();
}

void AutoReplaceList::getItems(vector<VoidP>& out) const {
	FOR_EACH_CONST(i, items) out.push_back(i);
}

void AutoReplaceList::sendEvent() {
	wxCommandEvent ev(EVENT_ITEM_SELECT, GetId());
	ProcessEvent(ev);
}

bool AutoReplaceList::compareItems(void* ap, void* bp) const {
	AutoReplace& a = *static_cast<AutoReplace*>(ap);
	AutoReplace& b = *static_cast<AutoReplace*>(bp);
	switch (sort_by_column) {
		case 2:  return a.replace < b.replace;
		default: return a.match   < b.match;
	}
}

String AutoReplaceList::OnGetItemText (long pos, long col) const {
	AutoReplaceP ar = getAR(pos);
	if (col == 0) return ar->match;
	if (col == 1) return ar->match;
	if (col == 2) return ar->replace;
	throw InternalError(_("too many columns"));
}

int AutoReplaceList::OnGetItemImage(long pos) const {
	return -1;
}

wxListItemAttr* AutoReplaceList::OnGetItemAttr(long pos) const {
	// grey disabled keywords
	return getAR(pos)->enabled ? nullptr : &item_attr;
}

// ----------------------------------------------------------------------------- : AutoReplaceWindow

AutoReplaceWindow::AutoReplaceWindow(Window* parent, const Game& game)
	: wxDialog(parent, wxID_ANY, _TITLE_("auto replaces"))
	, in_event(false)
{
	// Create controls
	list       = new AutoReplaceList(this, wxID_ANY, game);
	match      = new wxTextCtrl(this, ID_ITEM_VALUE);
	replace    = new wxTextCtrl(this, ID_ITEM_VALUE);
	enabled    = new wxCheckBox(this, ID_ITEM_VALUE, _BUTTON_("enabled"));
	whole_word = new wxCheckBox(this, ID_ITEM_VALUE, _BUTTON_("whole word"));
	use_auto_replace   = new wxCheckBox(this, ID_USE_AUTO_REPLACE, _BUTTON_("use auto replace"));
	add                = new wxButton(this, ID_ADD_ITEM,    _BUTTON_("add item"));
	remove             = new wxButton(this, ID_REMOVE_ITEM, _BUTTON_("remove item"));
	wxButton* defaults = new wxButton(this, ID_DEFAULTS,    _BUTTON_("defaults"));
	matchL = new wxStaticText(this, wxID_ANY, _LABEL_("auto match"));
	replaceL = new wxStaticText(this, wxID_ANY, _LABEL_("auto replace"));
	// Create sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		// enabled?
		s->Add(use_auto_replace, 0, wxALL & ~wxBOTTOM, 8);
		// list
		s->Add(list, 1, wxEXPAND | wxALL & ~wxBOTTOM, 8);
		s->AddSpacer(4);
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			s2->Add(add, 0, wxRIGHT, 4);
			s2->Add(remove);
		s->Add(s2, 0, wxALIGN_RIGHT | wxALL & ~wxTOP & ~wxBOTTOM, 8);
		// values
		wxFlexGridSizer* s3 = new wxFlexGridSizer(2);
			s3->AddGrowableCol(1);
			s3->Add(matchL,   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
			s3->Add(match,   1, wxEXPAND | wxBOTTOM, 2);
			s3->Add(replaceL, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
			s3->Add(replace, 1, wxEXPAND | wxBOTTOM, 2);
		s->Add(s3, 0, wxEXPAND | wxALL & ~wxBOTTOM, 8);
		s->AddSpacer(2);
		s->Add(whole_word, 0, wxALL & ~wxBOTTOM & ~wxTOP, 8);
		s->AddSpacer(4);
		s->Add(enabled,    0, wxALL & ~wxTOP & ~wxBOTTOM, 8);
		// buttons
		wxSizer* s4 = new wxBoxSizer(wxHORIZONTAL);
			s4->Add(defaults,                           0, wxALL & ~wxRIGHT, 8);
			s4->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxALL, 8);
		s->Add(s4, 0, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
	// Set default size
	SetSize(350, 450);
	// initialize values
	use_auto_replace->SetValue(list->gs.use_auto_replace);
	enable();
	refreshItem();
}

void AutoReplaceWindow::onItemSelect(wxCommandEvent&) {
	refreshItem();
}
void AutoReplaceWindow::onItemChange(wxCommandEvent&) {
	updateItem();
}

void AutoReplaceWindow::onEnable(wxCommandEvent&) {
	enable();
}

void AutoReplaceWindow::onOk(wxCommandEvent&) {
	store();
	Hide();
}

void AutoReplaceWindow::onRemove(wxCommandEvent&) {
	list->removeSelected();
}
void AutoReplaceWindow::onAdd(wxCommandEvent&) {
	list->addItem(new_intrusive<AutoReplace>());
}
void AutoReplaceWindow::onDefault(wxCommandEvent&) {
	use_auto_replace->SetValue(true);
	list->reset();
	enable();
}

void AutoReplaceWindow::enable() {
	bool enable = use_auto_replace->GetValue();
	list      ->Enable(enable);
	matchL    ->Enable(enable);
	replaceL  ->Enable(enable);
	match     ->Enable(enable);
	replace   ->Enable(enable);
	enabled   ->Enable(enable);
	whole_word->Enable(enable);
	add       ->Enable(enable);
	remove    ->Enable(enable);
}

void AutoReplaceWindow::refreshItem() {
	if (in_event) return;
	if (!use_auto_replace->GetValue()) return;
	in_event = true;
	AutoReplaceP ar = list->getSelected();
	match     ->Enable(ar && ar->custom);
	replace   ->Enable(ar);
	matchL    ->Enable(ar && ar->custom);
	replaceL  ->Enable(ar);
	enabled   ->Enable(ar);
	whole_word->Enable(ar);
	remove    ->Enable(ar && ar->custom);
	if (ar) {
		match     ->SetValue(ar->match);
		replace   ->SetValue(ar->replace);
		enabled   ->SetValue(ar->enabled);
		whole_word->SetValue(ar->whole_word);
	} else {
		match     ->SetValue(wxEmptyString);
		replace   ->SetValue(wxEmptyString);
		enabled   ->SetValue(false);
		whole_word->SetValue(false);
	}
	in_event = false;
}

void AutoReplaceWindow::updateItem() {
	if (in_event) return;
	in_event = true;
	AutoReplaceP ar = list->getSelected();
	if (!ar) return;
	ar->match      = match->GetValue();
	ar->replace    = replace->GetValue();
	ar->enabled    = enabled->GetValue();
	ar->whole_word = whole_word->GetValue();
	list->refreshList(true);
	in_event = false;
}

void AutoReplaceWindow::store() {
	list->gs.use_auto_replace = use_auto_replace->GetValue();
	swap(list->items, list->gs.auto_replaces);
}

BEGIN_EVENT_TABLE(AutoReplaceWindow, wxDialog)
	EVT_COMMAND		(wxID_ANY, EVENT_ITEM_SELECT, AutoReplaceWindow::onItemSelect)
	EVT_TEXT		(ID_ITEM_VALUE,  AutoReplaceWindow::onItemChange)
	EVT_CHECKBOX	(ID_ITEM_VALUE,  AutoReplaceWindow::onItemChange)
	EVT_CHECKBOX	(ID_USE_AUTO_REPLACE, AutoReplaceWindow::onEnable)
	EVT_BUTTON		(wxID_OK,        AutoReplaceWindow::onOk)
	EVT_BUTTON		(ID_ADD_ITEM,    AutoReplaceWindow::onAdd)
	EVT_BUTTON		(ID_REMOVE_ITEM, AutoReplaceWindow::onRemove)
	EVT_BUTTON		(ID_DEFAULTS,    AutoReplaceWindow::onDefault)
END_EVENT_TABLE()
