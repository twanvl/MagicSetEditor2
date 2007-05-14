//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/html_export_window.hpp>
#include <gui/control/package_list.hpp>
#include <data/export_template.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : HtmlExportWindow

HtmlExportWindow::HtmlExportWindow(Window* parent, const SetP& set)
	: wxDialog(parent,wxID_ANY,_TITLE_("export html"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxFULL_REPAINT_ON_RESIZE)
{
	// init controls
	list = new PackageList (this, wxID_ANY);
	// init sizers
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, _LABEL_("html template")), 0, wxALL, 4);
		s->Add(list, 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
		s->SetSizeHints(this);
	SetSizer(s);
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

BEGIN_EVENT_TABLE(HtmlExportWindow,wxDialog)
	EVT_BUTTON       (wxID_OK, HtmlExportWindow::onOk)
END_EVENT_TABLE  ()
