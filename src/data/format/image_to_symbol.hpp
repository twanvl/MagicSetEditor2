//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FORMAT_IMAGE_TO_SYMBOL
#define HEADER_DATA_FORMAT_IMAGE_TO_SYMBOL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/symbol.hpp>

// ----------------------------------------------------------------------------- : Image to symbol

/// Import an image as a symbol.
/** Handles MSE1 symbols by cutting out the symbol rectangle */
SymbolP import_symbol(Image& img);

/// Does the image represent a MSE1 symbol file?
/** Does some heuristic checks */
bool is_mse1_symbol(const Image& img);

/// Convert an image to a symbol, destroys the image in the process
SymbolP image_to_symbol(Image& img);

// ----------------------------------------------------------------------------- : Simplify symbol

/// Simplify a symbol
void simplify_symbol(Symbol&);

/// Simplify a symbol parts, i.e. use bezier curves instead of lots of lines
void simplify_symbol_shape(SymbolShape&);

// ----------------------------------------------------------------------------- : EOF
#endif
