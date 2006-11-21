//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_PARSER
#define HEADER_SCRIPT_PARSER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/script.hpp>

// ----------------------------------------------------------------------------- : Parser

/// Parse a String to a Script
/** If string_mode then s is interpreted as a string,
 *  escaping to script mode can be done with {}
 */
ScriptP parse(const String& s, bool string_mode = false);

// ----------------------------------------------------------------------------- : EOF
#endif
