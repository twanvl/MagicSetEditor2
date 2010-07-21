//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/html_export_window.hpp>
#include <gui/control/package_list.hpp>
#include <gui/control/native_look_editor.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/settings.hpp>
#include <data/export_template.hpp>
#include <util/window_id.hpp>
#include <util/error.hpp>
#include <util/platform.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>

DECLARE_POINTER_TYPE(ExportTemplate);

// ----------------------------------------------------------------------------- : HtmlExportWindow

HtmlExportWindow::HtmlExportWindow(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices)
	: ExportWindowBase(parent,_TITLE_("export html"), set, choices, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN)
	, set(set)
{
	// init controls
	list    = new PackageList(this, ID_EXPORT_LIST);
	options = new ExportOptionsEditor(this, wxID_ANY, wxNO_BORDER);
	options->setSet(intrusive(new Set(set->stylesheet))); // dummy set
	// init sizers
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("html template")), 0, wxALL, 4);
		s->Add(list, 0, wxEXPAND | (wxALL & ~wxTOP), 4);
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			s2->Add(ExportWindowBase::Create(), 2, wxEXPAND);
			wxSizer* s3 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("html export options"));
				s3->Add(options, 2, wxEXPAND, 0);
			s2->Add(s3, 7, wxEXPAND | wxLEFT, 8);
		s->Add(s2, 1, wxEXPAND | wxALL, 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
	SetSize(700,500);
	// list
	list->showData<ExportTemplate>(set->game->name() + _("-*"));
	list->select(settings.gameSettingsFor(*set->game).default_export);
}

void HtmlExportWindow::onOk(wxCommandEvent&) {
	ExportTemplateP exp = list->getSelection<ExportTemplate>();
	// get filename
	String name = wxFileSelector(_TITLE_("save html"),settings.default_export_dir,_(""),_(""),exp->file_type, wxSAVE | wxOVERWRITE_PROMPT);
	if (name.empty()) return;
	settings.default_export_dir = wxPathOnly(name);
	wxBusyCursor wait;
	// export info for script
	ExportInfo info;
	info.export_template = exp;
	info.set = set;
	WITH_DYNAMIC_ARG(export_info, &info);
	// create directory?
	if (exp->create_directory) {
		wxFileName fn(name);
		info.directory_relative = fn.GetName() + _("-files");
		fn.SetFullName(info.directory_relative);
		info.directory_absolute = fn.GetFullPath();
		if (!wxDirExists(info.directory_absolute)) {
			wxMkdir(info.directory_absolute);
		}
	}
	// run export script
	Context& ctx = set->getContext();
	LocalScope scope(ctx);
	ctx.setVariable(_("cards"),     to_script(&getSelection()));
	ctx.setVariable(_("options"),   to_script(&settings.exportOptionsFor(*exp)));
	ctx.setVariable(_("directory"), to_script(info.directory_relative));
	ScriptValueP result = exp->script.invoke(ctx);
	// Save to file
	wxFileOutputStream file(name);
	{ // TODO: write as image?
		// write as string
		wxTextOutputStream stream(file);
		stream.WriteString(*result);
	}
	// Done
	EndModal(wxID_OK);
}

void HtmlExportWindow::onTemplateSelect(wxCommandEvent&) {
	wxBusyCursor wait;
	ExportTemplateP export_template = list->getSelection<ExportTemplate>();
	handle_pending_errors();
	options->showExport(export_template);
	settings.gameSettingsFor(*set->game).default_export = export_template->name();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void HtmlExportWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case wxID_OK:
			ev.Enable(list->hasSelection() && !getSelection().empty());
			break;
	}
}

BEGIN_EVENT_TABLE(HtmlExportWindow,ExportWindowBase)
	EVT_GALLERY_SELECT (ID_EXPORT_LIST, HtmlExportWindow::onTemplateSelect)
	EVT_BUTTON         (wxID_OK,        HtmlExportWindow::onOk)
	EVT_UPDATE_UI      (wxID_ANY,       HtmlExportWindow::onUpdateUI)
END_EVENT_TABLE  ()
