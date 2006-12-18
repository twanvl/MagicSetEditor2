//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_PRINT_WINDOW
#define HEADER_GUI_PRINT_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

DECLARE_POINTER_TYPE(Set);

// ----------------------------------------------------------------------------- : Printing

/// Show a print preview for the given set
void print_preview(Window* parent, const SetP& set);

/// Print the given set
void print_set(Window* parent, const SetP& set);

// ----------------------------------------------------------------------------- : EOF
#endif
