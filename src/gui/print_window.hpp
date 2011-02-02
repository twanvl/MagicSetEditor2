//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_PRINT_WINDOW
#define HEADER_GUI_PRINT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/real_point.hpp>
#include <data/settings.hpp>
#include <gui/card_select_window.hpp>

DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(PrintJob);
class StyleSheet;

// ----------------------------------------------------------------------------- : Layout

/// Layout of a page of cards
class PageLayout {
  public:
	// layout
	RealSize page_size;			///< Size of a page (in millimetres)
	RealSize card_size;			///< Size of a card (in millimetres)
	RealSize card_spacing;		///< Spacing between cards (in millimetres)
	double margin_left, margin_right, margin_top, margin_bottom; ///< Page margins (in millimetres)
	int rows, cols;				///< Number of rows/columns of cards
	bool card_landscape;		///< Are cards rotated to landscape orientation?
	
	PageLayout();
	void init(const StyleSheet& stylesheet, PageLayoutType layout_type, const RealSize& page_size);
	
	/// Is this layout uninitialized?
	inline bool empty() const { return cards_per_page() == 0; }
	/// The number of cards per page
	inline int cards_per_page() const { return rows * cols; }
};

class PrintJob : public IntrusivePtrBase<PrintJob> {
  public:
	PrintJob(SetP const& set) : set(set) {}
	
	// set and cards to print
	SetP set;
	vector<CardP> cards;
	
	// printing options
	PageLayoutType layout_type;
	PageLayout layout;
	
	inline int num_pages() const {
		int cards_per_page = max(1,layout.cards_per_page());
		return (cards.size() + cards_per_page - 1) / cards_per_page;
	}
};

// ----------------------------------------------------------------------------- : Printing

/// Make a print job, by asking the user for options, and card selection
PrintJobP make_print_job(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices);

/// Show a print preview for the given set
void print_preview(Window* parent, const PrintJobP& job);
void print_preview(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices);

/// Print the given set
void print_set(Window* parent, const PrintJobP& job);
void print_set(Window* parent, const SetP& set, const ExportCardSelectionChoices& choices);

// ----------------------------------------------------------------------------- : EOF
#endif
