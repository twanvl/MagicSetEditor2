//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/keywords_panel.hpp>
#include <gui/control/keyword_list.hpp>
#include <gui/control/text_ctrl.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <data/keyword.hpp>
#include <data/game.hpp>
#include <data/action/value.hpp>
#include <data/action/keyword.hpp>
#include <data/field/text.hpp>
#include <util/window_id.hpp>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/artprov.h>

// ----------------------------------------------------------------------------- : KeywordsPanel

KeywordsPanel::KeywordsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	splitter  = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	list      = new KeywordList(splitter, wxID_ANY);
	panel     = new Panel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 /* no tab traversal*/);
	keyword   = new TextCtrl(panel, wxID_ANY, false);
	match     = new TextCtrl(panel, wxID_ANY, false);
	reminder  = new TextCtrl(panel, wxID_ANY, true); // allow multiline for wordwrap
	rules     = new TextCtrl(panel, wxID_ANY, true);
	errors    = new wxStaticText(panel, wxID_ANY, _(""));
	// warning about fixed keywords
	fixedL    = new wxStaticText(panel, wxID_ANY, _(""));
	wxStaticBitmap* fixedI = new wxStaticBitmap(panel, wxID_ANY, wxArtProvider::GetBitmap(wxART_WARNING));
	fixed = new wxBoxSizer(wxVERTICAL);
		wxSizer* s0 = new wxBoxSizer(wxHORIZONTAL);
		s0->Add(fixedI, 0, wxALIGN_CENTER | wxRIGHT, 10);
		s0->Add(fixedL, 0, wxALIGN_CENTER_VERTICAL);
	fixed->Add(new wxStaticLine(panel), 0, wxEXPAND | wxBOTTOM, 8);
	fixed->Add(s0, 0, wxALL & ~wxTOP | wxALIGN_CENTER, 8);
	fixed->Add(new wxStaticLine(panel), 0, wxEXPAND | wxBOTTOM, 8);
	// init sizer for panel
	sp = new wxBoxSizer(wxVERTICAL);
		sp->Add(fixed, 0, wxEXPAND); sp->Show(fixed,false);
		wxSizer* s1 = new wxBoxSizer(wxVERTICAL);
			s1->Add(new wxStaticText(panel, wxID_ANY, _("Keyword:")), 0);
			s1->Add(keyword, 0, wxEXPAND | wxTOP, 2);
		sp->Add(s1, 0, wxEXPAND | wxLEFT, 2);
		sp->Add(new wxStaticLine(panel), 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(new wxStaticText(panel, wxID_ANY, _("Match:")), 0);
			s2->Add(match, 0, wxEXPAND | wxTOP, 2);
			s2->Add(new wxStaticText(panel, wxID_ANY, _("Parameters:")), 0, wxTOP, 6);
		sp->Add(s2, 0, wxEXPAND | wxLEFT, 2);
		sp->Add(new wxStaticLine(panel), 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
		wxSizer* s3 = new wxBoxSizer(wxVERTICAL);
			s3->Add(new wxStaticText(panel, wxID_ANY, _("Reminder:")), 0);
			s3->Add(reminder, 1, wxEXPAND | wxTOP, 2);
			s3->Add(errors,   0, wxEXPAND | wxTOP, 4);
			s3->Add(new wxStaticText(panel, wxID_ANY, _("Example:")), 0, wxTOP, 6);
		sp->Add(s3, 1, wxEXPAND | wxLEFT, 2);
		sp->Add(new wxStaticLine(panel), 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
		wxSizer* s4 = new wxBoxSizer(wxVERTICAL);
			s4->Add(new wxStaticText(panel, wxID_ANY, _("Rules:")), 0);
			s4->Add(rules, 1, wxEXPAND | wxTOP, 2);
		sp->Add(s4, 1, wxEXPAND | wxLEFT, 2);
	panel->SetSizer(sp);
	// init splitter
	splitter->SetMinimumPaneSize(100);
	splitter->SetSashGravity(0.5);
	splitter->SplitVertically(list, panel);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->Add(splitter, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
	
	//s->Add(new wxStaticText(this, wxID_ANY, _("Sorry, no keywords for now"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTER), 1, wxALIGN_CENTER); // TODO: Remove
	/*	wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(list_active,   1, wxEXPAND);
			s2->Add(list_inactive, 1, wxEXPAND);*/
	
	// init menus
	menuKeyword = new IconMenu();
		menuKeyword->Append(ID_KEYWORD_PREV,						_MENU_("previous keyword"),		_HELP_("previous keyword"));
		menuKeyword->Append(ID_KEYWORD_NEXT,						_MENU_("next keyword"),			_HELP_("next keyword"));
		menuKeyword->AppendSeparator();
		menuKeyword->Append(ID_KEYWORD_ADD,		_("keyword_add"),	_MENU_("add keyword"),			_HELP_("add keyword"));
																	// NOTE: space after "Del" prevents wx from making del an accellerator
																	// otherwise we delete a card when delete is pressed inside the editor
		menuKeyword->Append(ID_KEYWORD_REMOVE,	_("keyword_del"),	_MENU_("remove keyword")+_(" "),_HELP_("remove keyword"));
}

KeywordsPanel::~KeywordsPanel() {
	delete menuKeyword;
}

// ----------------------------------------------------------------------------- : UI


void KeywordsPanel::initUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->AddTool(ID_KEYWORD_ADD,		_(""), load_resource_tool_image(_("keyword_add")),	wxNullBitmap, wxITEM_NORMAL,_TOOLTIP_("add keyword"),	_HELP_("add keyword"));
	tb->AddTool(ID_KEYWORD_REMOVE,	_(""), load_resource_tool_image(_("keyword_del")),	wxNullBitmap, wxITEM_NORMAL,_TOOLTIP_("remove keyword"),_HELP_("remove keyword"));
	tb->Realize();
	// Menus
	mb->Insert(2, menuKeyword,   _MENU_("keywords"));
}

void KeywordsPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	// Toolbar
	tb->DeleteTool(ID_KEYWORD_ADD);
	tb->DeleteTool(ID_KEYWORD_REMOVE);
	// Menus
	mb->Remove(2);
}

void KeywordsPanel::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case ID_KEYWORD_PREV:       ev.Enable(list->canSelectPrevious());	break;
		case ID_KEYWORD_NEXT:       ev.Enable(list->canSelectNext());		break;
		case ID_KEYWORD_REMOVE:     ev.Enable(list->getKeyword() && !list->getKeyword()->fixed);	break;
	}
}

void KeywordsPanel::onCommand(int id) {
	switch (id) {
		case ID_KEYWORD_PREV:
			list->selectPrevious();
			break;
		case ID_KEYWORD_NEXT:
			list->selectNext();
			break;
		case ID_KEYWORD_ADD:
			set->actions.add(new AddKeywordAction(*set));
			break;
		case ID_KEYWORD_REMOVE:
			if (!list->getKeyword()->fixed) {
				// only remove set keywords
				set->actions.add(new RemoveKeywordAction(*set, list->getKeyword()));
			}
			break;
	}
}


// ----------------------------------------------------------------------------- : Events

void KeywordsPanel::onChangeSet() {
	list->setSet(set);
	// warning label (depends on game name)
	fixedL->SetLabel(format_string(_LABEL_("standard keyword"), set->game->short_name));
	// init text controls
	keyword ->setSet(set);
	keyword ->getStyle().font.size = 16;
	keyword ->getStyle().font.font.SetPointSize(16);
	keyword ->getStyle().padding_bottom = 1;
	keyword ->updateSize();
	match   ->setSet(set);
	match   ->getStyle().font.size = 12;
	match   ->getStyle().font.font.SetPointSize(12);
	match   ->getStyle().padding_bottom = 1;
	match   ->updateSize();
	reminder->setSet(set);
	reminder->getStyle().padding_bottom = 2;
	match   ->getStyle().font.size = 10;
	match   ->getStyle().font.font.SetPointSize(10);
	reminder->updateSize();
	rules   ->setSet(set);
	// re-layout
	panel->Layout();
}

void KeywordsPanel::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, ValueAction) {
		KeywordReminderTextValue* value = dynamic_cast<KeywordReminderTextValue*>(action.valueP.get());
		if (value && &value->keyword == list->getKeyword().get()) {
			// the current keyword's reminder text changed
			errors->SetLabel(value->errors);
		}
	}
}

void KeywordsPanel::onKeywordSelect(KeywordSelectEvent& ev) {
	if (ev.keyword) {
		Keyword& kw = *ev.keyword;
		sp->Show(fixed, kw.fixed);
		keyword ->setValue(new_shared5<KeywordTextValue>        (keyword->getFieldP(),  &kw, &kw.keyword, !kw.fixed, true));
		match   ->setValue(new_shared4<KeywordTextValue>        (match->getFieldP(),    &kw, &kw.match,   !kw.fixed));
		rules   ->setValue(new_shared4<KeywordTextValue>        (rules->getFieldP(),    &kw, &kw.rules,   !kw.fixed));
		shared_ptr<KeywordReminderTextValue> reminder_value(new KeywordReminderTextValue(reminder->getFieldP(), &kw,              !kw.fixed));
		reminder->setValue(reminder_value);
		errors->SetLabel(reminder_value->errors);
		sp->Layout();
	} else {
		keyword ->setValue(nullptr);
		match   ->setValue(nullptr);
		rules   ->setValue(nullptr);
		reminder->setValue(nullptr);
	}
}

BEGIN_EVENT_TABLE(KeywordsPanel, wxPanel)
	EVT_KEYWORD_SELECT(wxID_ANY, KeywordsPanel::onKeywordSelect)
END_EVENT_TABLE()
