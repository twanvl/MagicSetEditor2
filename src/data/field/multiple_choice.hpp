//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_MULTIPLE_CHOICE
#define HEADER_DATA_FIELD_MULTIPLE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceField

DECLARE_POINTER_TYPE(MultipleChoiceField);
DECLARE_POINTER_TYPE(MultipleChoiceStyle);
DECLARE_POINTER_TYPE(MultipleChoiceValue);

/// A ChoiceField where multiple choices can be selected simultaniously
class MultipleChoiceField : public ChoiceField {
  public:
	MultipleChoiceField();
	DECLARE_FIELD_TYPE(MultipleChoiceField);
	
	UInt minimum_selection, maximum_selection; ///< How many choices can be selected simultaniously?
	String empty_choice; ///< Name to use when nothing is selected
};

// ----------------------------------------------------------------------------- : MultipleChoiceStyle

/// The Style for a MultipleChoiceField
class MultipleChoiceStyle : public ChoiceStyle {
  public:
	MultipleChoiceStyle(const MultipleChoiceFieldP& field);
	DECLARE_STYLE_TYPE(MultipleChoice);
	
	Direction direction;	///< In what direction are choices layed out?
	double spacing;			///< Spacing between choices (images) in pixels
};

// ----------------------------------------------------------------------------- : MultipleChoiceValue

/// The Value in a MultipleChoiceField
/** The value is stored as "<choice>, <choice>, <choice>"
 *  The choices must be ordered by id
 */
class MultipleChoiceValue : public ChoiceValue {
  public:
	inline MultipleChoiceValue(const MultipleChoiceFieldP& field) : ChoiceValue(field, false) {}
	DECLARE_HAS_FIELD(MultipleChoice);
	virtual ValueP clone() const;
	
	String last_change; ///< Which of the choices was selected/deselected last?
	
	// for SimpleValueAction
	struct ValueType {
		ChoiceValue::ValueType value;
		String                 last_change;
	};
	
	/// Splits the value, stores the selected choices in the out parameter
	void get(vector<String>& out) const;
	
	virtual bool update(Context&);
	
  private:
	DECLARE_REFLECTION();
	
	/// Put the value in normal form (all choices ordered, empty_name
	void normalForm();
};

// ----------------------------------------------------------------------------- : Utilities

/// Is the given choice selected in the value?
bool chosen(const String& multiple_choice_value, const String& chioce);

// ----------------------------------------------------------------------------- : EOF
#endif
