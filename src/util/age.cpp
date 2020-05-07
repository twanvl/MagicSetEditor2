//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/age.hpp>

// ----------------------------------------------------------------------------- : Age

// what a waste of a source file...

atomic<Age::age_t> Age::new_age(2);

IMPLEMENT_DYNAMIC_ARG(Age::age_t, last_update_age, 0);
