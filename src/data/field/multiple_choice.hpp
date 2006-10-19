//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_MULTIPLE_CHOICE
#define HEADER_DATA_FIELD_MULTIPLE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceField

DECLARE_POINTER_TYPE(MultipleChoiceField);

/// A ChoiceField where multiple choices can be selected simultaniously
class MultipleChoiceField : public ChoiceField {
  public:
	MultipleChoiceField();
	
	UInt minimum_selection, maximum_selection; ///< How many choices can be selected simultaniously?
	
	virtual ValueP newValue(const FieldP& thisP) const;
	virtual StyleP newStyle(const FieldP& thisP) const;
	virtual String typeName() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : MultipleChoiceStyle

enum Direction {
	HORIZONTAL, VERTICAL
};

/// The Style for a MultipleChoiceField
class MultipleChoiceStyle : public ChoiceStyle {
  public:
	MultipleChoiceStyle(const MultipleChoiceFieldP& field);
	HAS_FIELD(MultipleChoice)
	
	Direction direction;	///< In what direction are choices layed out?
	double spacing;			///< Spacing between choices (images) in pixels
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : MultipleChoiceValue

/// The Value in a MultipleChoiceField
/** The value is stored as "<choice>, <choice>, <choice>"
 *  The choices must be ordered by id
 */
class MultipleChoiceValue : public ChoiceValue {
  public:
	inline MultipleChoiceValue(const MultipleChoiceFieldP& field) : ChoiceValue(field) {}
	HAS_FIELD(MultipleChoice)
	
	// no extra data
	
	/// Splits the value, stores the selected choices in the out parameter
	void get(vector<String>& out);
	
  private:
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : EOF
#endif
