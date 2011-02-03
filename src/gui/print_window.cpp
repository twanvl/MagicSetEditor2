//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/print_window.hpp>
#include <gui/card_select_window.hpp>
#include <gui/util.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>
#include <render/card/viewer.hpp>
#include <wx/print.h>

DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_POINTER_TYPE(PageLayout);

// ----------------------------------------------------------------------------- : Layout

PageLayout::PageLayout()
	: margin_left(0), margin_right(0), margin_top(0), margin_bottom(0)
	, rows(0), cols(0), card_landscape(false)
{}

void PageLayout::init(const StyleSheet& stylesheet, PageLayoutType type, const RealSize& page_size) {
	this->page_size = page_size;
	margin_left = margin_right = margin_top = margin_bottom = 0;
	card_size.width  = stylesheet.card_width  * 25.4 / stylesheet.card_dpi;
	card_size.height = stylesheet.card_height * 25.4 / stylesheet.card_dpi;
	card_landscape = card_size.width > card_size.height;
	cols = int(floor(page_size.width  / card_size.width));
	rows = int(floor(page_size.height / card_size.height));
	// spacing
	double hspace = (page_size.width  - (cols * card_size.width ));
	double vspace = (page_size.height - (rows * card_size.height));
	if (type == LAYOUT_NO_SPACE) {
		// no space between cards
		card_spacing.width = card_spacing.height = 0;
		margin_left = margin_right  = hspace / 2;
		margin_top = vspace * 1./3; margin_bottom = vspace * 2./3; // most printers have more margin at the bottom
	} else {
		// distribute whitespace evenly
		margin_left = margin_right  = card_spacing.width  = hspace / (cols + 1);
		margin_top  = margin_bottom = card_spacing.height = vspace / (rows + 1);
	}
}

// ----------------------------------------------------------------------------- : Printout

/// A printout object specifying how to print a specified set of cards
class CardsPrintout : public wxPrintout {
  public:
	CardsPrintout(PrintJobP const& job);
	/// Number of pages, and something else I don't understand...
	virtual void GetPageInfo(int* pageMin, int* pageMax, int* pageFrom, int* pageTo);
	/// Again, 'number of pages', strange wx interface
	virtual bool HasPage(int page);
	/// Determine the layout
	virtual void OnPreparePrinting();
	/// Print a page
	virtual bool OnPrintPage(int page);
	
  private:
	PrintJobP job; ///< Cards to print
	DataViewer viewer;
	double scale_x, scale_y; // priter pixel per mm
	
	int pageCount() {
		return job->num_pages();
	}
	
	/// Draw a card, that is card_nr on this page, find the postion by asking the layout
	void drawCard(DC& dc, const CardP& card, int card_nr);
};

CardsPrintout::CardsPrintout(PrintJobP const& job)
	: job(job)
{
	viewer.setSet(job->set);
}

void CardsPrintout::GetPageInfo(int* page_min, int* page_max, int* page_from, int* page_to) {
	*page_from = *page_min = 1;
	*page_to   = *page_max = pageCount();
}

bool CardsPrintout::HasPage(int page) {
	return page <= pageCount(); // page number is 1 based
}

void CardsPrintout::OnPreparePrinting() {
	if (job->layout.empty()) {
		int pw_mm, ph_mm;
		GetPageSizeMM(&pw_mm, &ph_mm);
		job->layout.init(*job->set->stylesheet, job->layout_type, RealSize(pw_mm, ph_mm));
	}
}


bool CardsPrintout::OnPrintPage(int page) {
	DC& dc = *GetDC();
	// scale factors
	int pw_mm, ph_mm;
	GetPageSizeMM(&pw_mm, &ph_mm);
	int pw_px, ph_px;
	dc.GetSize(&pw_px, &ph_px);
	scale_x = (double)pw_px / pw_mm;
	scale_y = (double)ph_px / ph_mm;
	// print the cards that belong on this page
	int start = (page - 1) * job->layout.cards_per_page();
	int end   = min((int)job->cards.size(), start + job->layout.cards_per_page());
	for (int i = start ; i < end ; ++i) {
		drawCard(dc, job->cards.at(i), i - start);
	}
	return true;
}

void CardsPrintout::drawCard(DC& dc, const CardP& card, int card_nr) {
	// determine position
	int col = card_nr % job->layout.cols;
	int row = card_nr / job->layout.cols;
	RealPoint pos( job->layout.margin_left + (job->layout.card_size.width  + job->layout.card_spacing.width)  * col
	             , job->layout.margin_top  + (job->layout.card_size.height + job->layout.card_spacing.height) * row);
	// determine rotation
	const StyleSheet& stylesheet = job->set->stylesheetFor(card);
	int rotation = 0;
	if ((stylesheet.card_width > stylesheet.card_height) != job->layout.card_landscape) {
		rotation = 90 - rotation;
	}
	/*
	// size of this particular card (in mm)
	RealSize card_size( stylesheet.card_width  * 25.4 / stylesheet.card_dpi
	                  , stylesheet.card_height * 25.4 / stylesheet.card_dpi);
	if (rotation == 90) swap(card_size.width, card_size.height);
	// adjust card size, to center card in the available space (from job->layout.card_size)?
	// TODO: deal with different sized cards in general
	*/
	
	// create buffers
	int w = int(stylesheet.card_width), h = int(stylesheet.card_height); // in pixels
	if (rotation == 90) swap(w,h);
	// Draw using text buffer
	double zoom = IsPreview() ? 1 : 4;
	wxBitmap buffer(w*zoom,h*zoom,32);
	wxMemoryDC bufferDC;
	bufferDC.SelectObject(buffer);
	clearDC(bufferDC,*wxWHITE_BRUSH);
	RotatedDC rdc(bufferDC, rotation, stylesheet.getCardRect(), zoom, QUALITY_AA, ROTATION_ATTACH_TOP_LEFT);
	// render card to dc
	viewer.setCard(card);
	viewer.draw(rdc, *wxWHITE);
	// render buffer to device
	double px_per_mm = zoom * stylesheet.card_dpi / 25.4;
	dc.SetUserScale(scale_x / px_per_mm, scale_y / px_per_mm);
	dc.SetDeviceOrigin(int(scale_x * pos.x), int(scale_y * pos.y));
	bufferDC.SelectObject(wxNullBitmap);
	dc.DrawBitmap(buffer, 0, 0);
}

// ----------------------------------------------------------------------------- : PrintWindow

PrintJobP make_print_job(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices) {
	// Let the user choose cards
	// controls
	ExportWindowBase wnd(parent, _TITLE_("select cards"), set, choices);
	wxCheckBox* space = new wxCheckBox(&wnd, wxID_ANY, L"Put space between cards");
	space->SetValue(settings.print_layout);
	// layout
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			wxSizer* s3 = wnd.Create();
			s2->Add(s3, 1, wxEXPAND | wxALL, 8);
			wxSizer* s4 = new wxStaticBoxSizer(wxVERTICAL, &wnd, L"Settings");
				s4->Add(space, 1, wxALL | wxALIGN_TOP, 8);
			s2->Add(s4, 1, wxEXPAND | wxALL & ~wxLEFT, 8);
		s->Add(s2, 1, wxEXPAND);
		s->Add(wnd.CreateButtonSizer(wxOK | wxCANCEL) , 0, wxEXPAND | wxALL, 8);
	s->SetSizeHints(&wnd);
	wnd.SetSizer(s);
	wnd.SetMinSize(wxSize(300,-1));
	// show window
	if (wnd.ShowModal() != wxID_OK) {
		return PrintJobP(); // cancel
	} else {
		// make print job
		PrintJobP job = intrusive(new PrintJob(set));
		job->layout_type = settings.print_layout = space->GetValue() ? LAYOUT_EQUAL_SPACE : LAYOUT_NO_SPACE;
		job->cards = wnd.getSelection();
		return job;
	}
}

void print_preview(Window* parent, const PrintJobP& job) {
	if (!job) return;
	// Show the print preview
	wxPreviewFrame* frame = new wxPreviewFrame(
		new wxPrintPreview(
			new CardsPrintout(job),
			new CardsPrintout(job)
		), parent, _TITLE_("print preview"));
	frame->Initialize();
	frame->Maximize(true);
	frame->Show();
}

void print_set(Window* parent, const PrintJobP& job) {
	if (!job) return;
	// Print the cards
	wxPrinter p;
	CardsPrintout pout(job);
	p.Print(parent, &pout, true);
}

void print_preview(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices) {
	print_preview(parent, make_print_job(parent, set, choices));
}
void print_set(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices) {
	print_set(parent, make_print_job(parent, set, choices));
}
