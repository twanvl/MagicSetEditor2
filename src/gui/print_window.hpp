//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_PRINT_WINDOW
#define HEADER_GUI_PRINT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/real_point.hpp>

DECLARE_POINTER_TYPE(Set);
class StyleSheet;

// ----------------------------------------------------------------------------- : Printing

/// Show a print preview for the given set
void print_preview(Window* parent, const SetP& set);

/// Print the given set
void print_set(Window* parent, const SetP& set);

// ----------------------------------------------------------------------------- : Layout

/// Layout of a page of cards
class PageLayout {
  public:
	PageLayout();
	PageLayout(const StyleSheet& stylesheet, const RealSize& page_size);
	
	RealSize page_size;			///< Size of a page (in millimetres)
	RealSize card_size;			///< Size of a card (in millimetres)
	RealSize card_spacing;		///< Spacing between cards (in millimetres)
	double margin_left, margin_right, margin_top, margin_bottom; ///< Page margins (in millimetres)
	int rows, cols;				///< Number of rows/columns of cards
	bool card_landscape;		///< Are cards rotated to landscape orientation?
	
	/// The number of cards per page
	inline int cardsPerPage() const { return rows * cols; }
	
 private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
