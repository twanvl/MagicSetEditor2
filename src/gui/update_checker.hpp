//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_UPDATE_CHECKER
#define HEADER_UTIL_UPDATE_CHECKER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Update checking

// Checks for updates if the settings say so
void check_updates();

/// Checks if the current version is the latest version
/** If async==true then checking is done in another thread
 */
void check_updates_now(bool async = true);

/// Show a dialog to inform the user that updates are availible (if there are any)
/** Call check_updates first.
 *  Call this function from an onIdle loop */
void show_update_dialog(Window* parent);

/// Was update data found?
bool update_data_found();
/// Is there an update?
bool update_available();

// ----------------------------------------------------------------------------- : EOF
#endif
