//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD
#define HEADER_DATA_FIELD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

#ifndef HEADER_DATA_CARD
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Value);
#endif

// ----------------------------------------------------------------------------- : Field

class Field {
  public:
	UInt index; // used by IndexMap
};

class Value {
};

void initObject(const FieldP&, ValueP&);

// ----------------------------------------------------------------------------- : EOF
#endif
