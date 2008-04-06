//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_FUNCTIONS_FUNCTIONS
#define HEADER_SCRIPT_FUNCTIONS_FUNCTIONS

/** @file script/functions/functions.cpp
 *
 *  @brief Header for buildin script functions.
 */
 
// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class Context;

// ----------------------------------------------------------------------------- : Script functions

void init_script_basic_functions(Context& ctx);
void init_script_image_functions(Context& ctx);
void init_script_editor_functions(Context& ctx);
void init_script_export_functions(Context& ctx);
void init_script_english_functions(Context& ctx);

/// Initialize all build in functions for a context
inline void init_script_functions(Context& ctx) {
	init_script_basic_functions(ctx);
	init_script_image_functions(ctx);
	init_script_editor_functions(ctx);
	init_script_export_functions(ctx);
	init_script_english_functions(ctx);
}

// ----------------------------------------------------------------------------- : EOF
#endif
