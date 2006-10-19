//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_SYMBOL
#define HEADER_DATA_FIELD_SYMBOL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(SymbolFilter);

// ----------------------------------------------------------------------------- : SymbolField

/// A field for image values
class SymbolField : public Field {
  public:
	// no extra data
	
	virtual ValueP newValue(const FieldP& thisP) const;
	virtual StyleP newStyle(const FieldP& thisP) const;
	virtual String typeName() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : SymbolStyle

/// The Style for a SymbolField
class SymbolStyle : public Style {
  public:
	class Variation;
	typedef shared_ptr<Variation> VariationP;
	vector<VariationP> variations; ///< Different variantions of the same symbol
	
  private:
	DECLARE_REFLECTION();
};

/// Styling for a symbol variation, defines color, border, etc.
class SymbolStyle::Variation {
  public:
	Variation();
	String        name;				///< Name of this variation
	SymbolFilterP filter;			///< Filter to color the symbol
	double        border_radius;	///< Border radius for the symbol
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : SymbolValue

/// The Value in a SymbolField, i.e. a symbol
class SymbolValue : public Value {
  public:
	String filename; ///< Filename of the symbol (in the current package)
	
	virtual String toString() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
