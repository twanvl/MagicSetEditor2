//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/preferences_window.hpp>
#include <gui/update_checker.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <util/io/package_manager.hpp>
#include <wx/spinctrl.h>
#include <wx/filename.h>

// ----------------------------------------------------------------------------- : Preferences pages

// A page from the preferences dialog
class PreferencesPage : public wxPanel {
  public:
	PreferencesPage(Window* parent)
		: wxPanel(parent, wxID_ANY)
	{}
	
	/// Stores the settings from the panel in the global settings object
	virtual void store() = 0;
};

// Preferences page for global MSE settings
class GlobalPreferencesPage : public PreferencesPage {
  public:
	GlobalPreferencesPage(Window* parent);
	void store();	
	
  private:
	wxComboBox* language;
};

// Preferences page for card viewing related settings
class DisplayPreferencesPage : public PreferencesPage {
  public:
	DisplayPreferencesPage(Window* parent);
	void store();	
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxCheckBox* high_quality, *borders;
	wxSpinCtrl* zoom;
	wxCheckBox* non_normal_export;
	
	void onSelectColumns(wxCommandEvent&);
};

// Preferences page for directories of programs
// i.e. Apprentice, Magic Workstation
// perhaps in the future also directories for packages?
class DirsPreferencesPage : public PreferencesPage {
  public:
	DirsPreferencesPage(Window* parent);
	void store();
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxTextCtrl* apprentice;
	
	void onApprenticeBrowse(wxCommandEvent&);
};

// Preferences page for automatic updates
class UpdatePreferencesPage : public PreferencesPage {
  public:
	UpdatePreferencesPage(Window* parent);
	void store();
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxChoice* check_at_startup;
	
	// check for updates
	void onCheckUpdatesNow(wxCommandEvent&);
};


// ----------------------------------------------------------------------------- : PreferencesWindow

PreferencesWindow::PreferencesWindow(Window* parent)
	: wxDialog(parent, wxID_ANY, _TITLE_("preferences"), wxDefaultPosition)
{
	// init notebook
	wxNotebook* nb = new wxNotebook(this, ID_NOTEBOOK);
	nb->AddPage(new GlobalPreferencesPage (nb), _TITLE_("global"));
	nb->AddPage(new DisplayPreferencesPage(nb), _TITLE_("display"));
	nb->AddPage(new DirsPreferencesPage   (nb), _TITLE_("directories"));
	nb->AddPage(new UpdatePreferencesPage (nb), _TITLE_("updates"));
	
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(nb,                                 1, wxEXPAND | wxALL & ~wxBOTTOM, 8);
	s->AddSpacer(4);
	s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP,    8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void PreferencesWindow::onOk(wxCommandEvent&) {
	// store each page
	wxNotebook* nb = static_cast<wxNotebook*>(FindWindow(ID_NOTEBOOK));
	size_t count = nb->GetPageCount();
	for (size_t i = 0 ; i < count ; ++i) {
		static_cast<PreferencesPage*>(nb->GetPage(i))->store();
	}
	// close
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(PreferencesWindow, wxDialog)
	EVT_BUTTON       (wxID_OK, PreferencesWindow::onOk)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Preferences page : global

GlobalPreferencesPage::GlobalPreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	language = new wxComboBox(this, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY | wxCB_SORT);
	// set values
	String f = ::packages.findFirst(_("*.mse-locale"));
	int n = 0;
	while (!f.empty()) {
		PackagedP package = packages.openAny(f);
		language->Append(package->name() + _(": ") + package->full_name, package.get());
		if (settings.locale == package->name()) {
			language->SetSelection(n);
		}
		n++;
		f = wxFindNextFile();
	}
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->SetSizeHints(this);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("language"));
			s2->Add(new wxStaticText(this, wxID_ANY, _LABEL_("app language")), 0,            wxALL,          4);
			s2->Add(language,                                                  0, wxEXPAND | wxALL & ~wxTOP, 4);
			s2->Add(new wxStaticText(this, wxID_ANY, _HELP_( "app language")), 0,            wxALL & ~wxTOP, 4);
		s->Add(s2, 0, wxALL | wxEXPAND, 8);
	SetSizer(s);
}

void GlobalPreferencesPage::store() {
	int n = language->GetSelection();
	if (n == wxNOT_FOUND) return;
	Packaged* p = (Packaged*)language->GetClientData(n);
	settings.locale = p->name();
	// set the_locale?
}

// ----------------------------------------------------------------------------- : Preferences page : display

DisplayPreferencesPage::DisplayPreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	high_quality      = new wxCheckBox(this, wxID_ANY, _BUTTON_("high quality"));
	borders           = new wxCheckBox(this, wxID_ANY, _BUTTON_("show lines"));
	zoom              = new wxSpinCtrl(this, wxID_ANY);
	non_normal_export = new wxCheckBox(this, wxID_ANY, _BUTTON_("zoom export"));
	//wxButton* columns = new wxButton(this, ID_SELECT_COLUMNS, _BUTTON_("select"));
	// set values
	high_quality->     SetValue( settings.default_stylesheet_settings.card_anti_alias());
	borders->          SetValue( settings.default_stylesheet_settings.card_borders());
	non_normal_export->SetValue(!settings.default_stylesheet_settings.card_normal_export());
	zoom->SetRange(1, 1000);
	zoom->             SetValue(static_cast<int>(settings.default_stylesheet_settings.card_zoom() * 100));
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("card display"));
			s2->Add(high_quality, 0, wxEXPAND | wxALL, 4);
			s2->Add(borders,      0, wxEXPAND | wxALL, 4);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				s3->Add(new wxStaticText(this, wxID_ANY, _LABEL_("zoom")),             0, wxALL & ~wxLEFT,  4);
				s3->Add(zoom);
				s3->Add(new wxStaticText(this, wxID_ANY, _LABEL_("percent of normal")),1, wxALL & ~wxRIGHT, 4);
			s2->Add(s3, 0, wxEXPAND | wxALL, 4);
			s2->Add(non_normal_export,0, wxEXPAND | wxALL, 4);
			s2->Add(new wxStaticText(this, wxID_ANY,     _HELP_("zoom export")),       0, wxALL & ~wxTOP, 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
		/*
		wxSizer* s4 = new wxStaticBoxSizer(wxVERTICAL, this, _("Card List"));
			wxSizer* s5 = new wxBoxSizer(wxHORIZONTAL);
				s5->Add(new wxStaticText(this, wxID_ANY, _("Columns: ")),      0, wxALL & ~wxLEFT | wxEXPAND, 4);
				s5->Add(columns);
			s4->Add(s5, 0, wxEXPAND | wxALL, 4);
		s->Add(s4, 0, wxEXPAND | wxALL & ~wxTOP, 8);
		*/
	s->SetSizeHints(this);
	SetSizer(s);
}

void DisplayPreferencesPage::store() {
	settings.default_stylesheet_settings.card_anti_alias    = high_quality->GetValue();
	settings.default_stylesheet_settings.card_borders       = borders->GetValue();
	settings.default_stylesheet_settings.card_zoom          = zoom->GetValue() / 100.0;
	settings.default_stylesheet_settings.card_normal_export = !non_normal_export->GetValue();
}

void DisplayPreferencesPage::onSelectColumns(wxCommandEvent&) {
	// Impossible, set specific
}

BEGIN_EVENT_TABLE(DisplayPreferencesPage, wxPanel)
	EVT_BUTTON       (ID_SELECT_COLUMNS, DisplayPreferencesPage::onSelectColumns)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Preferences page : directories

DirsPreferencesPage::DirsPreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	apprentice   = new wxTextCtrl(this, wxID_ANY);
	wxButton* ab = new wxButton(this, ID_APPRENTICE_BROWSE, _BUTTON_("browse"));
	// set values
	apprentice->SetValue(settings.apprentice_location);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("external programs"));
			s2->Add(new wxStaticText(this, wxID_ANY, _LABEL_("apprentice")), 0, wxALL, 4);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				s3->Add(apprentice, 1, wxEXPAND | wxRIGHT, 4);
				s3->Add(ab,         0, wxEXPAND);
			s2->Add(s3, 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void DirsPreferencesPage::store() {
	settings.apprentice_location = apprentice->GetValue();
}

void DirsPreferencesPage::onApprenticeBrowse(wxCommandEvent&) {
	// browse for appr.exe
	wxFileDialog dlg(this, _TITLE_("locate apprentice"), apprentice->GetValue(), _(""), _LABEL_("apprentice exe") + _("|appr.exe"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileName fn(dlg.GetPath());
		apprentice->SetValue(fn.GetPath());
	}
}
	
BEGIN_EVENT_TABLE(DirsPreferencesPage, wxPanel)
	EVT_BUTTON     (ID_APPRENTICE_BROWSE, DirsPreferencesPage::onApprenticeBrowse)
END_EVENT_TABLE  ();


// ----------------------------------------------------------------------------- : Preferences page : updates

UpdatePreferencesPage::UpdatePreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	check_at_startup    = new wxChoice(this, wxID_ANY);
	wxButton* check_now = new wxButton(this, ID_CHECK_UPDATES_NOW, _BUTTON_("check now"));
	// set values
	check_at_startup->Append(_BUTTON_("always"));                        // 0
	check_at_startup->Append(_BUTTON_("if internet connection exists")); // 1
	check_at_startup->Append(_BUTTON_("never"));                         // 2
	check_at_startup->SetSelection(settings.check_updates);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("check at startup")), 0, wxALL, 8);
		s->Add(check_at_startup, 0, wxALL & ~wxTOP, 8);
		s->Add(check_now,        0, wxALL & ~wxTOP, 8);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("checking requires internet")), 0, wxALL & ~wxTOP, 8);
	SetSizer(s);
}

void UpdatePreferencesPage::store() {
	int sel = check_at_startup->GetSelection();
	if      (sel == 0) settings.check_updates = CHECK_ALWAYS;
	else if (sel == 1) settings.check_updates = CHECK_IF_CONNECTED;
	else               settings.check_updates = CHECK_NEVER;
}

void UpdatePreferencesPage::onCheckUpdatesNow(wxCommandEvent&) {
	check_updates_now(false);
	if (!update_data_found()) {
		wxMessageBox(_ERROR_("checking updates failed"), _TITLE_("update check"), wxICON_ERROR | wxOK);
	} else if (!update_available()) {
		wxMessageBox(_ERROR_("no updates"),              _TITLE_("update check"), wxICON_INFORMATION | wxOK);
	} else {
		show_update_dialog(GetParent());
	}
}

BEGIN_EVENT_TABLE(UpdatePreferencesPage, wxPanel)
	EVT_BUTTON      (ID_CHECK_UPDATES_NOW, UpdatePreferencesPage::onCheckUpdatesNow)
END_EVENT_TABLE  ()
