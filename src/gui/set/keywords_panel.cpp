//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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

DECLARE_TYPEOF_COLLECTION(ParamReferenceTypeP);
DECLARE_TYPEOF_COLLECTION(KeywordParamP);
DECLARE_TYPEOF_COLLECTION(KeywordModeP);

// ----------------------------------------------------------------------------- : KeywordsPanel

KeywordsPanel::KeywordsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	splitter  = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	list      = new KeywordList(splitter, wxID_ANY);
	panel     = new Panel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 /* no tab traversal*/);
	keyword   = new TextCtrl(panel, ID_KEYWORD,  false);
	match     = new TextCtrl(panel, ID_MATCH,    false);
	reminder  = new TextCtrl(panel, ID_REMINDER, true); // allow multiline for wordwrap
	rules     = new TextCtrl(panel, ID_RULES,    true);
	errors    = new wxStaticText(panel, wxID_ANY, _(""));
	errors->SetForegroundColour(*wxRED);
	mode      = new wxChoice(panel, ID_KEYWORD_MODE, wxDefaultPosition, wxDefaultSize, 0, nullptr);
	add_param = new wxButton(panel, ID_KEYWORD_ADD_PARAM, _BUTTON_("insert parameter"));
	ref_param = new wxButton(panel, ID_KEYWORD_REF_PARAM, _BUTTON_("refer parameter"));
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
			s1->Add(new wxStaticText(panel, wxID_ANY, _("Mode:")), 0, wxTOP, 2);
			s1->Add(mode, 0, wxEXPAND | wxTOP, 2);
		sp->Add(s1, 0, wxEXPAND | wxLEFT, 2);
		sp->Add(new wxStaticLine(panel), 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(new wxStaticText(panel, wxID_ANY, _("Match:")), 0);
			s2->Add(match, 0, wxEXPAND | wxTOP, 2);
			s2->Add(add_param, 0, wxALIGN_LEFT | wxTOP, 2);
		sp->Add(s2, 0, wxEXPAND | wxLEFT, 2);
		sp->Add(new wxStaticLine(panel), 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
		wxSizer* s3 = new wxBoxSizer(wxVERTICAL);
			s3->Add(new wxStaticText(panel, wxID_ANY, _("Reminder:")), 0);
			s3->Add(reminder, 1, wxEXPAND | wxTOP, 2);
			s3->Add(ref_param, 0, wxALIGN_LEFT | wxTOP, 2);
			s3->Add(errors,   0, wxEXPAND | wxTOP, 4);
			//s3->Add(new wxStaticText(panel, wxID_ANY, _("Example:")), 0, wxTOP, 6);
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
			set->actions.add(new AddKeywordAction(ADD, *set));
			break;
		case ID_KEYWORD_REMOVE:
			if (!list->getKeyword()->fixed) {
				// only remove set keywords
				set->actions.add(new AddKeywordAction(REMOVE, *set, list->getKeyword()));
			}
			break;
		case ID_KEYWORD_ADD_PARAM: {
			wxMenu param_menu;
			int id = ID_PARAM_TYPE_MIN;
			FOR_EACH(p, set->game->keyword_parameter_types) {
				param_menu.Append(id++, p->name, p->description);
			}
			add_param->PopupMenu(&param_menu, 0, add_param->GetSize().y);
			break;
		}
		case ID_KEYWORD_REF_PARAM: {
			wxMenu ref_menu;
			int id = ID_PARAM_REF_MIN;
			int param = 0;
			FOR_EACH(p, list->getKeyword()->parameters) {
				String item = String() << ++param << _(". ") << LEFT_ANGLE_BRACKET << p->name << RIGHT_ANGLE_BRACKET;
				if (p->refer_scripts.empty()) {
					ref_menu.Append(id++, item);
				} else {
					wxMenu* submenu = new wxMenu();
					FOR_EACH(r, p->refer_scripts) {
						submenu->Append(id++, r->name, r->description);
					}
					ref_menu.Append(wxID_ANY, item, submenu);
				}
			}
			ref_param->PopupMenu(&ref_menu, 0, ref_param->GetSize().y);
			break;
		}
		default:
			if (id >= ID_PARAM_TYPE_MIN && id < ID_PARAM_TYPE_MAX) {
				// add parameter
				KeywordParamP param = set->game->keyword_parameter_types.at(id - ID_PARAM_TYPE_MIN);
				String to_insert = _("<atom-param>") + param->name + _("</atom-param>");
				match->insert(to_insert, _("Insert parameter"));
			} else if (id >= ID_PARAM_REF_MIN && id < ID_PARAM_REF_MAX) {
				String to_insert = runRefScript(id - ID_PARAM_REF_MIN);
				reminder->insert(to_insert, _("Use parameter"));
			}
	}
}

String KeywordsPanel::runRefScript(int find_i) {
	int param = 0;
	int i = 0;
	FOR_EACH(p, list->getKeyword()->parameters) {
		String param_s = String(_("param")) << ++param;
		if (p->refer_scripts.empty()) {
			if (i++ == find_i) {
				// found it
				return _("{") + param_s + _("}");
			}
		} else {
			FOR_EACH(r, p->refer_scripts) {
				if (i++ == find_i) {
					Context& ctx = set->getContext();
					ctx.setVariable(_("input"), to_script(param_s));
					return r->script.invoke(ctx)->toString();
				}
			}
		}
	}
	return wxEmptyString;
}

// ----------------------------------------------------------------------------- : Clipboard

// determine what control to use for clipboard actions
#define CUT_COPY_PASTE(op,return)													\
	int id = focused_control(this);													\
	if      (id == ID_KEYWORD  && keyword ->IsEnabled()) { return keyword ->op(); }	\
	else if (id == ID_MATCH    && match   ->IsEnabled()) { return match   ->op(); }	\
	else if (id == ID_REMINDER && reminder->IsEnabled()) { return reminder->op(); }	\
	else if (id == ID_RULES    && rules   ->IsEnabled()) { return rules   ->op(); }	\
	else                                                 { return false;          }

bool KeywordsPanel::canCopy()  const { CUT_COPY_PASTE(canCopy,  return) }
bool KeywordsPanel::canCut()   const { if (!list->getKeyword() || list->getKeyword()->fixed) return false;
                                       CUT_COPY_PASTE(canCut,   return) }
bool KeywordsPanel::canPaste() const { if (!list->getKeyword() || list->getKeyword()->fixed) return false;
                                       CUT_COPY_PASTE(canPaste, return) }
void KeywordsPanel::doCopy()         { CUT_COPY_PASTE(doCopy,  ;) }
void KeywordsPanel::doCut()          { if (!list->getKeyword() || list->getKeyword()->fixed) return;
                                       CUT_COPY_PASTE(doCut,   ;) }
void KeywordsPanel::doPaste()        { if (!list->getKeyword() || list->getKeyword()->fixed) return;
                                       CUT_COPY_PASTE(doPaste, ;) }

// ----------------------------------------------------------------------------- : Events

void KeywordsPanel::onChangeSet() {
	list->setSet(set);
	// warning label (depends on game name)
	fixedL->SetLabel(format_string(_LABEL_("standard keyword"), set->game->short_name));
	// init text controls
	keyword ->setSet(set);
	keyword ->getStyle().font.size = 16;
	keyword ->getStyle().padding_bottom = 1;
	keyword ->updateSize();
	match   ->setSet(set);
	match   ->getStyle().font.size = 12;
	match   ->getStyle().padding_bottom = 1;
	match   ->updateSize();
	reminder->setSet(set);
	reminder->getStyle().padding_bottom = 2;
	match   ->getStyle().font.size = 10;
	reminder->updateSize();
	rules   ->setSet(set);
	// parameter & mode lists
	add_param->Enable(false);
	ref_param->Enable(false);
	mode->Clear();
	FOR_EACH(m, set->game->keyword_modes) {
		mode->Append(m->name);
	}
	mode     ->Enable(false);
	// re-layout
	panel->Layout();
}

void KeywordsPanel::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, ValueAction) {
		{
			KeywordReminderTextValue* value = dynamic_cast<KeywordReminderTextValue*>(action.valueP.get());
			if (value && &value->keyword == list->getKeyword().get()) {
				// the current keyword's reminder text changed
				errors->SetLabel(value->errors);
			}
		}
		{
			KeywordTextValue* value = dynamic_cast<KeywordTextValue*>(action.valueP.get());
			if (value && value->underlying == &list->getKeyword()->match) {
				// match string changes, maybe there are parameters now
				ref_param->Enable(!value->keyword.fixed && !value->keyword.parameters.empty());
			}
		}
	}
	TYPE_CASE(action, ChangeKeywordModeAction) {
		if (&action.keyword == list->getKeyword().get()) {
			// the current keyword's mode changed
			mode->SetSelection((int)list->getKeyword()->findMode(set->game->keyword_modes));
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
		add_param->Enable(!kw.fixed && !set->game->keyword_parameter_types.empty());
		ref_param->Enable(!kw.fixed && !kw.parameters.empty());
		mode     ->Enable(!kw.fixed && !set->game->keyword_modes.empty());
		mode->SetSelection((int)kw.findMode(set->game->keyword_modes));
		sp->Layout();
	} else {
		keyword ->setValue(nullptr);
		match   ->setValue(nullptr);
		rules   ->setValue(nullptr);
		reminder->setValue(nullptr);
		add_param->Enable(false);
		ref_param->Enable(false);
		mode     ->Enable(false);
	}
}

void KeywordsPanel::onModeChange(wxCommandEvent& ev) {
	if (!list->getKeyword()) return;
	int sel = mode->GetSelection();
	if (sel >= 0 && (size_t)sel < set->game->keyword_modes.size()) {
		set->actions.add(new ChangeKeywordModeAction(*list->getKeyword(), set->game->keyword_modes[sel]->name));
	}
}


BEGIN_EVENT_TABLE(KeywordsPanel, wxPanel)
	EVT_KEYWORD_SELECT(wxID_ANY,        KeywordsPanel::onKeywordSelect)
	EVT_CHOICE        (ID_KEYWORD_MODE, KeywordsPanel::onModeChange)
END_EVENT_TABLE()
