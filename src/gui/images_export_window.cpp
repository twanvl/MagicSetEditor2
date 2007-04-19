//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

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

ImagesExportWindow::ImagesExportWindow(Window* parent, const SetP& set)
	: CardSelectWindow(parent, set, wxEmptyString, _TITLE_("select cards export"), false)
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
			s2->Add(format,                                                    0, wxEXPAND | wxALL & ~wxTOP, 4);
			s2->Add(new wxStaticText(this, -1, _HELP_("filename format")),     0, wxALL & ~wxTOP, 4);
			s2->Add(new wxStaticText(this, -1, _LABEL_("filename conflicts")), 0, wxALL, 4);
			s2->Add(conflicts,                                                 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
		wxSizer* s3 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("cards to export"));
			s3->Add(list, 1, wxEXPAND | wxALL, 4);
			wxSizer* s4 = new wxBoxSizer(wxHORIZONTAL);
				s4->Add(sel_all,  0, wxEXPAND | wxRIGHT, 4);
				s4->Add(sel_none, 0, wxEXPAND,           4);
			s3->Add(s4, 0, wxEXPAND | wxALL & ~wxTOP, 8);	
		s->Add(s3, 1, wxEXPAND | wxALL & ~wxTOP, 8);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP, 8);
	s->SetSizeHints(this);
	SetSizer(s);
	SetSize(500,500);
}

// ----------------------------------------------------------------------------- : Exporting the images

bool is_filename_char(Char c) {
	return isAlnum(c) || c == _(' ') || c == _('_') || c == _('-') || c == _('.');
}

void ImagesExportWindow::onOk(wxCommandEvent&) {
	// Update settings
	GameSettings& gs = settings.gameSettingsFor(*set->game);
	gs.images_export_filename  = format->GetValue();
	int sel = conflicts->GetSelection();
	if      (sel == 0) gs.images_export_conflicts = CONFLICT_KEEP_OLD;
	else if (sel == 1) gs.images_export_conflicts = CONFLICT_OVERWRITE;
	else if (sel == 2) gs.images_export_conflicts = CONFLICT_NUMBER;
	else               gs.images_export_conflicts = CONFLICT_NUMBER_OVERWRITE;
	// Script
	ScriptP filename_script = parse(gs.images_export_filename, true);
	// Select filename
	String name = wxFileSelector(_TITLE_("export images"),_(""), _LABEL_("filename is ignored"),_(""),
		                         _LABEL_("filename is ignored")+_("|*.*"), wxSAVE, this);
	if (name.empty()) return;
	wxFileName fn(name);
	// Export
	std::set<String> used; // for CONFLICT_NUMBER_OVERWRITE
	FOR_EACH(card, set->cards) {
		if (isSelected(card)) {
			// filename for this card
			Context& ctx = set->getContext(card);
			String filename = untag(ctx.eval(*filename_script)->toString());
			if (!filename) continue; // no filename -> no saving
			// sanitize filename
			String clean_filename;
			FOR_EACH(c, filename) {
				if (is_filename_char(c)) {
					clean_filename += c;
				}
			}
			if (clean_filename.empty() || starts_with(clean_filename, _("."))) {
				clean_filename = _("no-name") + clean_filename;
			}
			fn.SetFullName(clean_filename);
			// does the file exist?
			if (fn.FileExists()) {
				// file exists, what to do?
				switch (gs.images_export_conflicts) {
					case CONFLICT_KEEP_OLD:  goto next_card;
					case CONFLICT_OVERWRITE: break;
					case CONFLICT_NUMBER: {
						int i = 0;
						String ext = fn.GetExt();
						do {
							fn.SetExt(String() << ++i << _(".") << ext);
						} while(fn.FileExists());
					}
					case CONFLICT_NUMBER_OVERWRITE: {
						int i = 0;
						String ext = fn.GetExt();
						while(used.find(fn.GetFullPath()) != used.end()) {
							fn.SetExt(String() << ++i << _(".") << ext);
						}
					}
				}
			}
			// write image
			filename = fn.GetFullPath();
			used.insert(filename);
			export_image(set, card, filename);
		}
		next_card:;
	}
	// Done
	EndModal(wxID_OK);
}


BEGIN_EVENT_TABLE(ImagesExportWindow,CardSelectWindow)
	EVT_BUTTON       (wxID_OK,  ImagesExportWindow::onOk)
END_EVENT_TABLE  ()
