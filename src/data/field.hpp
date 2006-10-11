//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD
#define HEADER_DATA_FIELD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>

DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);
DECLARE_POINTER_TYPE(Value);

// ----------------------------------------------------------------------------- : Field

/// Information on how to store a value
class Field {
  public:
	Field();
	virtual ~Field();
	
	UInt      index;            ///< Used by IndexMap
	String    name;             ///< Name of the field, for refering to it from scripts and files
	String    description;      ///< Description, used in status bar
	bool      editable;         ///< Can values of this field be edited?
	bool      save_value;       ///< Should values of this field be written to files? Can be false for script generated fields.
	bool      show_statistics;  ///< Should this field appear as a group by choice in the statistics panel?
	bool      identifying;      ///< Does this field give Card::identification()?
	int       card_list_column; ///< What column to use in the card list? -1 = don't list
	UInt      card_list_width;  ///< Width of the card list column (pixels).
	bool      card_list_allow;  ///< Is this field allowed to appear in the card list.
	String    card_list_name;   ///< Alternate name to use in card list.
//	Alignment card_list_align;  ///< Alignment of the card list colummn.
	int       tab_index;        ///< Tab index in editor
//	Vector<DependendScript> dependendScripts; // scripts that depend on values of this field
	
	/// Creates a new Value corresponding to this Field
	/** thisP is a smart pointer to this */
	virtual ValueP newValue(const FieldP& thisP) const = 0;
	/// Creates a new Style corresponding to this Field
	/** thisP is a smart pointer to this */
	virtual StyleP newStyle(const FieldP& thisP) const = 0;
	/// create a copy of this field
	virtual FieldP clone() const = 0;
	/// Type of this field
	virtual String typeName() const = 0;
	
  private:
	DECLARE_REFLECTION_VIRTUAL();
};

template <>
shared_ptr<Field> read_new<Field>(Reader& reader);


// ----------------------------------------------------------------------------- : Style

/// Style information needed to display a Value in a Field.
class Style {
  public:
	virtual ~Style();
	
  private:
	DECLARE_REFLECTION_VIRTUAL();
};

void initObject(const FieldP&, StyleP&);

// ----------------------------------------------------------------------------- : Value

/// A specific value 'in' a Field.
class Value {
  public:
	virtual ~Value();
	
	/// Create a copy of this value
	virtual ValueP clone() const = 0;
		
  private:
	DECLARE_REFLECTION_VIRTUAL();
};

void initObject(const FieldP&, ValueP&);

// ----------------------------------------------------------------------------- : EOF
#endif
