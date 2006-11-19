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
#include <util/alignment.hpp>
#include <util/age.hpp>
#include <script/scriptable.hpp>
#include <script/dependency.hpp>

DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);
DECLARE_POINTER_TYPE(Value);
class Context;
class Dependency;

// for DataViewer/editor
class DataViewer; class DataEditor;
DECLARE_POINTER_TYPE(ValueViewer);
DECLARE_POINTER_TYPE(ValueEditor);

// ----------------------------------------------------------------------------- : Field

/// Information on how to store a value
class Field {
  public:
	Field();
	virtual ~Field();
	
	size_t    index;            ///< Used by IndexMap
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
	Alignment card_list_align;  ///< Alignment of the card list colummn.
	int       tab_index;        ///< Tab index in editor
	vector<Dependency> dependent_scripts; ///< Scripts that depend on values of this field
	
	/// Creates a new Value corresponding to this Field
	/** thisP is a smart pointer to this */
	virtual ValueP newValue(const FieldP& thisP) const = 0;
	/// Creates a new Style corresponding to this Field
	/** thisP is a smart pointer to this */
	virtual StyleP newStyle(const FieldP& thisP) const = 0;
	/// Type of this field
	virtual String typeName() const = 0;
	
	/// Add the given dependency to the dependet_scripts list for the variables this field depends on
	virtual void initDependencies(Context&, const Dependency&) const {}
	
  private:
	DECLARE_REFLECTION_VIRTUAL();
};

template <>
shared_ptr<Field> read_new<Field>(Reader& reader);
inline void update_index(FieldP& f, size_t index) {
	f->index = index;
}

// ----------------------------------------------------------------------------- : Style

/// Style information needed to display a Value in a Field.
class Style {
  public:
	Style(const FieldP&);
	virtual ~Style();
	
	const FieldP       fieldP;			///< Field this style is for, should have the right type!
	int                z_index;			///< Stacking of values of this field, higher = on top
	Scriptable<double> left,  top;		///< Position of this field
	Scriptable<double> width, height;	///< Position of this field
	Scriptable<bool>   visible;			///< Is this field visible?
	
	inline RealPoint getPos()  const { return RealPoint(left, top               ); }
	inline RealSize  getSize() const { return RealSize (           width, height); }
	inline RealRect  getRect() const { return RealRect (left, top, width, height); }
	
	/// Make a viewer object for values using this style
	/** thisP is a smart pointer to this */
	virtual ValueViewerP makeViewer(DataViewer& parent, const StyleP& thisP) = 0;
	/// Make an editor object for values using this style
	/** thisP is a smart pointer to this */
	virtual ValueViewerP makeEditor(DataEditor& parent, const StyleP& thisP) = 0;
	
	/// Update scripted values of this style, return true if anything has changed
	virtual bool update(Context&);
	/// Add the given dependency to the dependent_scripts list for the variables this style depends on
	virtual void initDependencies(Context&, const Dependency&) const;
	
  private:
	DECLARE_REFLECTION_VIRTUAL();
};

void init_object(const FieldP&, StyleP&);
inline const FieldP& get_key     (const StyleP& s) { return s->fieldP; }
inline const String& get_key_name(const StyleP& s) { return s->fieldP->name; }
template <> StyleP read_new<Style>(Reader&);

// ----------------------------------------------------------------------------- : Value

/// A specific value 'in' a Field.
class Value {
  public:
	inline Value(const FieldP& field) : fieldP(field) {}
	virtual ~Value();
	
	const FieldP fieldP;				///< Field this value is for, should have the right type!
	Age          last_script_update;	///< When where the scripts last updated? (by calling update)
	
	/// Convert this value to a string for use in tables
	virtual String toString() const = 0;
	/// Apply scripts to this value, return true if the value has changed
	virtual bool update(Context&) { last_script_update.update(); return false; }
	
  private:
	DECLARE_REFLECTION_VIRTUAL();
};

void init_object(const FieldP&, ValueP&);
inline const FieldP& get_key     (const ValueP& v) { return v->fieldP; }
inline const String& get_key_name(const ValueP& v) { return v->fieldP->name; }
template <> ValueP read_new<Value>(Reader&);

// ----------------------------------------------------------------------------- : Utilities

#define DECLARE_FIELD_TYPE(Type)														\
	virtual ValueP newValue(const FieldP& thisP) const;									\
	virtual StyleP newStyle(const FieldP& thisP) const;									\
	virtual String typeName() const

// implement newStyle and newValue
#define IMPLEMENT_FIELD_TYPE(Type)														\
	StyleP Type ## Field::newStyle(const FieldP& thisP) const {							\
		assert(thisP.get() == this);													\
		return new_shared1<Type ## Style>(static_pointer_cast<Type ## Field>(thisP));	\
	}																					\
	ValueP Type ## Field::newValue(const FieldP& thisP) const {							\
		assert(thisP.get() == this);													\
		return new_shared1<Type ## Value>(static_pointer_cast<Type ## Field>(thisP));	\
	}

#define DECLARE_STYLE_TYPE(Type)														\
	DECLARE_HAS_FIELD(Type)																\
	virtual ValueViewerP makeViewer(DataViewer& parent, const StyleP& thisP);			\
	virtual ValueViewerP makeEditor(DataEditor& parent, const StyleP& thisP)

// implement field() which returns a field with the right (derived) type
#define DECLARE_HAS_FIELD(Type)															\
	inline Type ## Field& field() const {												\
		return *static_cast<Type ## Field*>(fieldP.get());								\
	}

// ----------------------------------------------------------------------------- : EOF
#endif
