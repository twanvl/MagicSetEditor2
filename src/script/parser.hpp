//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_PARSER
#define HEADER_SCRIPT_PARSER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/error.hpp>
#include <script/script.hpp>

class Packaged;

// ----------------------------------------------------------------------------- : Parser

/// Parse a String to a Script
/** If string_mode then s is interpreted as a string,
 *  escaping to script mode can be done with {}.
 *
 *  Errors are stored in the output vector.
 *  If there are errors, the result is a null pointer
 *
 *  The package is for loading included files, it may be null
 */
ScriptP parse(const String& s, Packaged* package, bool string_mode, vector<ScriptParseError>& errors_out);

/// Parse a String to a Script
/** If string_mode then s is interpreted as a string,
 *  escaping to script mode can be done with {}.
 *
 *  If an error is encountered, an exception is thrown.
 */
ScriptP parse(const String& s, Packaged* package = nullptr, bool string_mode = false);

// ----------------------------------------------------------------------------- : EOF
#endif
