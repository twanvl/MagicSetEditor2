//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/images_export_window.hpp>
#include <gui/control/select_card_list.hpp>
#include <data/settings.hpp>
#include <data/set.hpp>
#include <data/format/formats.hpp>
#include <script/parser.hpp>
#include <script/context.hpp>
#include <util/tagged_string.hpp>
#include <wx/filename.h>

DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : ImagesExportWindow

ImagesExportWindow::ImagesExportWindow(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices)
	: ExportWindowBase(parent, _TITLE_("select cards export"), set, choices)
{
	// init controls
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	format    = new wxTextCtrl(this, wxID_ANY, gs.images_export_filename);
	conflicts = new wxChoice  (this, wxID_ANY);
	conflicts->Append(_BUTTON_("keep old"));         // 0
	conflicts->Append(_BUTTON_("overwrite"));        // 1
	conflicts->Append(_BUTTON_("number"));           // 2
	conflicts->Append(_BUTTON_("number overwrite")); // 3
	conflicts->SetSelection(gs.images_export_conflicts);
	// init sizers
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("export filenames"));
			s2->Add(new wxStaticText(this, -1, _LABEL_("filename format")),    0, wxALL, 4);
			s2->Add(format,                                                    0, wxEXPAND | (wxALL & ~wxTOP), 4);
			s2->Add(new wxStaticText(this, -1, _HELP_("filename format")),     0, wxALL & ~wxTOP, 4);
			s2->Add(new wxStaticText(this, -1, _LABEL_("filename conflicts")), 0, wxALL, 4);
			s2->Add(conflicts,                                                 0, wxEXPAND | (wxALL & ~wxTOP), 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
		wxSizer* s3 = ExportWindowBase::Create();
		s->Add(s3, 1, wxEXPAND | (wxALL & ~wxTOP), 8);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | (wxALL & ~wxTOP), 8);
	s->SetSizeHints(this);
	SetSizer(s);
	SetSize(500,-1);
}

// ----------------------------------------------------------------------------- : Exporting the images

void ImagesExportWindow::onOk(wxCommandEvent&) {
	// Update settings
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	gs.images_export_filename  = format->GetValue();
	int sel = conflicts->GetSelection();
	if      (sel == 0) gs.images_export_conflicts = CONFLICT_KEEP_OLD;
	else if (sel == 1) gs.images_export_conflicts = CONFLICT_OVERWRITE;
	else if (sel == 2) gs.images_export_conflicts = CONFLICT_NUMBER;
	else               gs.images_export_conflicts = CONFLICT_NUMBER_OVERWRITE;
	// Select filename
	String name = wxFileSelector(_TITLE_("export images"), settings.default_export_dir, _LABEL_("filename is ignored"),_(""),
		                         _LABEL_("filename is ignored")+_("|*"), wxFD_SAVE, this);
	if (name.empty()) return;
	settings.default_export_dir = wxPathOnly(name);
	// Export
	export_images(set, getSelection(), name, gs.images_export_filename, gs.images_export_conflicts);
	// Done
	EndModal(wxID_OK);
}


BEGIN_EVENT_TABLE(ImagesExportWindow,ExportWindowBase)
	EVT_BUTTON       (wxID_OK,  ImagesExportWindow::onOk)
END_EVENT_TABLE  ()
