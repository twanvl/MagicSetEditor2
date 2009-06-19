//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/age.hpp>

// ----------------------------------------------------------------------------- : Age

// what a waste of a source file...

AtomicInt Age::new_age(2);

IMPLEMENT_DYNAMIC_ARG(AtomicIntEquiv, last_update_age, 0);
