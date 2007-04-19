//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/print_window.hpp>
#include <gui/card_select_window.hpp>
#include <gui/util.hpp>
#include <data/set.hpp>
#include <wx/print.h>

DECLARE_TYPEOF_COLLECTION(CardP);

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
	TextBufferDC(UInt width, UInt height);
	
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
		
		TextDraw(wxFont font, Color color, int x, int y, String text, double angle = 0)
			: font(font), color(color), x(x), y(y), text(text), angle(angle)
		{}
	};
  public:
	typedef shared_ptr<TextDraw> TextDrawP;
  private:
	vector<TextDrawP> text;
	Bitmap buffer;
};

TextBufferDC::TextBufferDC(UInt width, UInt height)
	: buffer(width, height, 32)
{
	SelectObject(buffer);
	// initialize to white
	clearDC(*this,*wxWHITE_BRUSH);
}
void TextBufferDC::DoDrawText(const String& str, int x, int y) {
	text.push_back( new_shared5<TextDraw>(GetFont(), GetTextForeground(), x, y, str) );
}
void TextBufferDC::DoDrawRotatedText(const String& str, int x, int y, double angle) {
	text.push_back( new_shared6<TextDraw>(GetFont(), GetTextForeground(), x, y, str, angle) );
}

DECLARE_TYPEOF_COLLECTION(TextBufferDC::TextDrawP);

void TextBufferDC::drawToDevice(DC& dc, int x, int y) {
	SelectObject(wxNullBitmap);
	dc.DrawBitmap(buffer, x, y);
	FOR_EACH(t, text) {
		dc.SetFont          (t->font);
		dc.SetTextForeground(t->color);
		if (t->angle) {
			dc.DrawRotatedText(t->text, t->x + x, t->y + y, t->angle);
		} else {
			dc.DrawText(t->text, t->x + x, t->y + y);
		}
	}
}

// ----------------------------------------------------------------------------- : Layout

/// Layout of a page of cards
class PageLayout {
  public:
	RealSize card_size;			///< Size of a card
	RealSize card_space;		///< Spacing between cards
	double margin_left, margin_right, margin_top, margin_bottom; ///< Page margins
	int rows, cols;				///< Number of rows/columns of cards
	bool landscape;				///< Are cards rotated to landscape orientation?
};

// ----------------------------------------------------------------------------- : Printout

/// A printout object specifying how to print a specified set of cards
class CardsPrintout : wxPrintout {
  public:
	CardsPrintout(const SetP& set, const vector<CardP>& cards);
	/// Determine card size, cards per row
	void OnPreparePrinting();
	/// Number of pages, and something else I don't understand...
	void GetPageInfo(int* pageMin, int* pageMax, int* pageFrom, int* pageTo);
	/// Again, 'number of pages', strange wx interface
	bool HasPage(int page);
	/// Print a page
	bool OnPrintPage(int page);
	
  private:
	PageLayout layout;
	
	/// Draw a card, that is card_nr on this page, find the postion by asking the layout
	void drawCard(DC& dc, const CardP& card, size_t card_nr);
	/// Draw a card at the specified coordinates
	void drawCard(DC& dc, const CardP& card, double x, double y, int rotation = 0);
};

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
/*	// Show the print preview
	wxPreviewFrame frame = new wxPreviewFrame(
		new wxPrintPreview(
			new CardsPrintout(set, selected),
			new CardsPrintout(set, selected)
		), parent, _TITLE_("print preview"));
	frame->Initialize();
	frame->Maximize(true);
	frame->Show();
*/
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
/*	// Print the cards
	wxPrinter p;
	CardsPrintout pout(set, selected);
	p.Print(parent, &pout, true);
*/
}
