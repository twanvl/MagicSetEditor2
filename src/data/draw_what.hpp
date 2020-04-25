//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_DRAW_WHAT
#define HEADER_DATA_DRAW_WHAT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : DrawWhat

/// What should be drawn?
enum DrawWhat
{  DRAW_NOTHING    = 0x00
,  DRAW_NORMAL     = 0x01  // draw normal things, like the text
,  DRAW_BORDERS    = 0x10  // draw editor stuff, such as borders/lines, can be disabled.
,  DRAW_BOXES      = 0x20  // draw editor stuff, such as borders/lines, can be disabled.
,  DRAW_EDITING    = 0x40  // draw other editor stuff, can be disabled.
,  DRAW_ERRORS     = 0x80  // draw error indicators, can't be disabled
,  DRAW_ACTIVE     = 0x100 // draw active editor stuff, such as hidden separators and atom highlights
,  DRAW_HOVER      = 0x200 // draw mouse over stuff, such as a highlighted border
,  DRAW_NATIVELOOK = 0x400 // use a native look
};

// ----------------------------------------------------------------------------- : EOF
#endif
