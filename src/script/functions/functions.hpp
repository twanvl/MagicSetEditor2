//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
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
void init_script_regex_functions(Context& ctx);
void init_script_image_functions(Context& ctx);
void init_script_editor_functions(Context& ctx);
void init_script_export_functions(Context& ctx);
void init_script_english_functions(Context& ctx);
void init_script_spelling_functions(Context& ctx);
void init_script_construction_functions(Context& ctx);

/// Initialize all built in functions for a context
inline void init_script_functions(Context& ctx) {
	init_script_basic_functions(ctx);
	init_script_regex_functions(ctx);
	init_script_image_functions(ctx);
	init_script_editor_functions(ctx);
	init_script_export_functions(ctx);
	init_script_english_functions(ctx);
	init_script_spelling_functions(ctx);
	init_script_construction_functions(ctx);
}

// ----------------------------------------------------------------------------- : EOF
#endif
