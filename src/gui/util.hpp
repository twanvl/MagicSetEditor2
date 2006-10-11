//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_UTIL
#define HEADER_GUI_UTIL

/** @file gui/util.hpp
 *  Utility functions for use in the gui. Most are related to drawing.
 */
 
// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : DC related

/// Fill a DC with a single color
void clearDC(DC& dc, const wxBrush& brush = *wxBLACK_BRUSH);

// ----------------------------------------------------------------------------- : Resource related

/// Load an image from a resource
Image load_resource_image(String name);

// ----------------------------------------------------------------------------- : EOF
#endif
