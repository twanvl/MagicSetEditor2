//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/html_export_window.hpp>
#include <gui/control/package_list.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/settings.hpp>
#include <data/export_template.hpp>
#include <util/window_id.hpp>
#include <util/error.hpp>

DECLARE_POINTER_TYPE(ExportTemplate);

// ----------------------------------------------------------------------------- : HtmlExportWindow

HtmlExportWindow::HtmlExportWindow(Window* parent, const SetP& set)
	: wxDialog(parent,wxID_ANY,_TITLE_("export html"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxFULL_REPAINT_ON_RESIZE)
	, set(set)
{
	// init controls
	list = new PackageList(this, ID_EXPORT_LIST);
	// init sizers
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("html template")), 0, wxALL, 4);
		s->Add(list, 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
	// list
	list->showData<ExportTemplate>(set->game->name() + _("-*"));
	list->select(settings.gameSettingsFor(*set->game).default_export);
}

void HtmlExportWindow::onOk(wxCommandEvent&) {
	handle_error(Error(_("HTML export is not implemented yet, sorry")));
	/*;//%%
	String name = fileSelector(_("Exort to html"),_(""),_(""),_(""), {
		                        _("HTML files (*.html)|*.html"),
		                        wxSAVE | wxOVERWRITE_PROMPT);
	}
	if (!name.empty()) {
		HtmlExportWindow wnd(&this, set, name);
		wnd.showModal();
	}
	*/
	// Done
	EndModal(wxID_OK);
}

void HtmlExportWindow::onTemplateSelect(wxCommandEvent&) {
	wxBusyCursor wait;
	ExportTemplateP export = list->getSelection<ExportTemplate>();
	handle_pending_errors();
	settings.gameSettingsFor(*set->game).default_export = export->name();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void HtmlExportWindow::onUpdateUI(wxUpdateUIEvent& ev) {
	switch (ev.GetId()) {
		case wxID_OK:
			ev.Enable(list->hasSelection());
			break;
	}
}

BEGIN_EVENT_TABLE(HtmlExportWindow,wxDialog)
	EVT_GALLERY_SELECT (ID_EXPORT_LIST, HtmlExportWindow::onTemplateSelect)
	EVT_BUTTON         (wxID_OK,        HtmlExportWindow::onOk)
	EVT_UPDATE_UI      (wxID_ANY,       HtmlExportWindow::onUpdateUI)
END_EVENT_TABLE  ()
