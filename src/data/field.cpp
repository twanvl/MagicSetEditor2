//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field.hpp>

// ----------------------------------------------------------------------------- : Field
// ----------------------------------------------------------------------------- : Value

void initObject(const FieldP& field, ValueP& value) {
	value = new_shared<Value>();
}
