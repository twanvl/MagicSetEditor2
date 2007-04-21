//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/print_window.hpp>
#include <gui/card_select_window.hpp>
#include <gui/util.hpp>
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <render/card/viewer.hpp>
#include <wx/print.h>

DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_POINTER_TYPE(PageLayout);

// ----------------------------------------------------------------------------- : Buffering DC

/// MemoryDC that buffers calls to write text
/** The printer device doesn't support alpha channels (at least not in wxMSW)
 *  This would result in black backgrounds where symbols should be transparent
 *  Our solution is:
 *   1. Write all bitmaps to a buffer DC, initially white
 *   2. When drawing with alpha: blend with the buffer
 *   3. When drawing text: buffer the call
 *   4. Draw the buffer image to the device
 *   5. Replay the buffered text draw calls
 *  To simplify things this class itself is a fullblown DC, only text calls are buffered for later
 *  Actually buffering text separatly would not be necessary at all, but if we don't the text will be
 *  printed in a low resolution.
 */
class TextBufferDC : public wxMemoryDC {
  public:
	TextBufferDC(int width, int height);
	
	virtual void DoDrawText(const String& str, int x, int y);
	virtual void DoDrawRotatedText(const String& str, int x, int y, double angle);
	
	/// Copy the contents of the DC to a target device, this DC becomes invalid
	void drawToDevice(DC& dc, int x = 0, int y = 0);
	
  private:
	// A call to DrawText
	struct TextDraw {
		wxFont font;
		Color  color;
		int x, y;
		String text;
		double angle;
		double user_scale_x, user_scale_y;
		
		TextDraw(wxFont font, Color color, double user_scale_x, double user_scale_y, int x, int y, String text, double angle = 0)
			: font(font), color(color), user_scale_x(user_scale_x), user_scale_y(user_scale_y), x(x), y(y), text(text), angle(angle)
		{}
	};
  public:
	typedef shared_ptr<TextDraw> TextDrawP;
  private:
	vector<TextDrawP> text;
	Bitmap buffer;
};

TextBufferDC::TextBufferDC(int width, int height)
	: buffer(width, height, 32)
{
	SelectObject(buffer);
	// initialize to white
	clearDC(*this,*wxWHITE_BRUSH);
}
void TextBufferDC::DoDrawText(const String& str, int x, int y) {
	double usx,usy;
	GetUserScale(&usx, &usy);
	text.push_back( new_shared7<TextDraw>(GetFont(), GetTextForeground(), usx, usy, x, y, str) );
}
void TextBufferDC::DoDrawRotatedText(const String& str, int x, int y, double angle) {
	double usx,usy;
	GetUserScale(&usx, &usy);
	text.push_back( new_shared8<TextDraw>(GetFont(), GetTextForeground(), usx, usy, x, y, str, angle) );
}

DECLARE_TYPEOF_COLLECTION(TextBufferDC::TextDrawP);

void TextBufferDC::drawToDevice(DC& dc, int x, int y) {
	SelectObject(wxNullBitmap);
	dc.DrawBitmap(buffer, x, y);
	FOR_EACH(t, text) {
		double usx,usy;
		dc.GetUserScale(&usx, &usy);
		dc.SetUserScale(usx * t->user_scale_x, usx * t->user_scale_y);
		dc.SetFont          (t->font);
		dc.SetTextForeground(t->color);
		if (t->angle) {
			dc.DrawRotatedText(t->text, t->x + x, t->y + y, t->angle);
		} else {
			dc.DrawText(t->text, t->x + x, t->y + y);
		}
		dc.SetUserScale(usx, usy);
	}
}

// ----------------------------------------------------------------------------- : Layout

PageLayout::PageLayout()
	: margin_left(0), margin_right(0), margin_top(0), margin_bottom(0)
	, rows(0), cols(0), card_landscape(false)
{}

PageLayout::PageLayout(const StyleSheet& stylesheet, const RealSize& page_size)
	: page_size(page_size)
	, margin_left(0), margin_right(0), margin_top(0), margin_bottom(0)
{
	card_size.width  = stylesheet.card_width  * 25.4 / stylesheet.card_dpi;
	card_size.height = stylesheet.card_height * 25.4 / stylesheet.card_dpi;
	card_landscape = card_size.width > card_size.height;
	cols = floor(page_size.width  / card_size.width);
	rows = floor(page_size.height / card_size.height);
	// distribute whitespace evenly
	margin_left = margin_right  = card_spacing.width  = (page_size.width  - (cols * card_size.width )) / (cols + 1);
	margin_top  = margin_bottom = card_spacing.height = (page_size.height - (rows * card_size.height)) / (rows + 1);
}

// ----------------------------------------------------------------------------- : Printout

/// A printout object specifying how to print a specified set of cards
class CardsPrintout : public wxPrintout {
  public:
	CardsPrintout(const SetP& set, const vector<CardP>& cards);
	/// Number of pages, and something else I don't understand...
	virtual void GetPageInfo(int* pageMin, int* pageMax, int* pageFrom, int* pageTo);
	/// Again, 'number of pages', strange wx interface
	virtual bool HasPage(int page);
	/// Determine the layout
	virtual void OnPreparePrinting();
	/// Print a page
	virtual bool OnPrintPage(int page);
	
  private:
	PageLayoutP layout;
	SetP set;
	vector<CardP> cards; ///< Cards to print
	DataViewer viewer;
	double scale_x, scale_y; // priter pixel per mm
	
	inline int pageCount() {
		return ((int)cards.size() + layout->cardsPerPage() - 1) / layout->cardsPerPage();
	}
	
	/// Draw a card, that is card_nr on this page, find the postion by asking the layout
	void drawCard(DC& dc, const CardP& card, int card_nr);
};

CardsPrintout::CardsPrintout(const SetP& set, const vector<CardP>& cards)
	: set(set), cards(cards)
{
	viewer.setSet(set);
}

void CardsPrintout::GetPageInfo(int* page_min, int* page_max, int* page_from, int* page_to) {
	*page_from = *page_min = 1;
	*page_to   = *page_max = pageCount();
}

bool CardsPrintout::HasPage(int page) {
	return page <= pageCount(); // page number is 1 based
}

void CardsPrintout::OnPreparePrinting() {
	int pw_mm, ph_mm;
	GetPageSizeMM(&pw_mm, &ph_mm);
	if (!layout) {
		layout = new_shared2<PageLayout>(*set->stylesheet, RealSize(pw_mm, ph_mm));
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
	int start = (page - 1) * layout->cardsPerPage();
	int end   = min((int)cards.size(), start + layout->cardsPerPage());
	for (int i = start ; i < end ; ++i) {
		drawCard(dc, cards[i], i - start);
	}
	return true;
}

void CardsPrintout::drawCard(DC& dc, const CardP& card, int card_nr) {
	// determine position
	int col = card_nr % layout->cols;
	int row = card_nr / layout->cols;
	RealPoint pos( layout->margin_left + (layout->card_size.width  + layout->card_spacing.width)  * col
	             , layout->margin_top  + (layout->card_size.height + layout->card_spacing.height) * row);
	// determine rotation
	StyleSheet& stylesheet = *set->stylesheetFor(card);
	int rotation = 0;
	if ((stylesheet.card_width > stylesheet.card_height) != layout->card_landscape) {
		rotation = 90 - rotation;
	}
	/*
	// size of this particular card (in mm)
	RealSize card_size( stylesheet.card_width  * 25.4 / stylesheet.card_dpi
	                  , stylesheet.card_height * 25.4 / stylesheet.card_dpi);
	if (rotation == 90) swap(card_size.width, card_size.height);
	// adjust card size, to center card in the available space (from layout->card_size)?
	// TODO
	*/
	
	// create buffers
	int w = stylesheet.card_width, h = stylesheet.card_height; // in pixels
	if (rotation == 90) swap(w,h);
	TextBufferDC bufferDC(w,h);
	RotatedDC rdc(bufferDC, rotation, RealRect(0,0,w,h), 1.0, QUALITY_SUB_PIXEL);
	// render card to dc
	viewer.setCard(card);
	viewer.draw(rdc, *wxWHITE);
	// render buffer to device
	double px_per_mm = stylesheet.card_dpi / 25.4;
	dc.SetUserScale(scale_x / px_per_mm, scale_y / px_per_mm);
	dc.SetDeviceOrigin(scale_x * pos.x, scale_y * pos.y);
	bufferDC.drawToDevice(dc, 0, 0); // adjust for scaling
}

// ----------------------------------------------------------------------------- : PrintWindow

void print_preview(Window* parent, const SetP& set) {
	// Let the user choose cards
	CardSelectWindow wnd(parent, set, _LABEL_("select cards print"), _TITLE_("select cards"));
	if (wnd.ShowModal() != wxID_OK) {
		return; // cancel
	}
	vector<CardP> selected;
	FOR_EACH(c, set->cards) {
		if (wnd.isSelected(c)) selected.push_back(c);
	}
	// Show the print preview
	wxPreviewFrame* frame = new wxPreviewFrame(
		new wxPrintPreview(
			new CardsPrintout(set, selected),
			new CardsPrintout(set, selected)
		), parent, _TITLE_("print preview"));
	frame->Initialize();
	frame->Maximize(true);
	frame->Show();
}

void print_set(Window* parent, const SetP& set) {
	// Let the user choose cards
	CardSelectWindow wnd(parent, set, _LABEL_("select cards print"), _TITLE_("select cards"));
	if (wnd.ShowModal() != wxID_OK) {
		return; // cancel
	}
	vector<CardP> selected;
	FOR_EACH(c, set->cards) {
		if (wnd.isSelected(c)) selected.push_back(c);
	}
	// Print the cards
	wxPrinter p;
	CardsPrintout pout(set, selected);
	p.Print(parent, &pout, true);
}
