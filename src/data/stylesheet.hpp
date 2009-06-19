//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_STYLESHEET
#define HEADER_DATA_STYLESHEET

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <util/real_point.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);

// ----------------------------------------------------------------------------- : StyleSheet

/// Stylesheet of the set that is currently being read/written
DECLARE_DYNAMIC_ARG(StyleSheet*, stylesheet_for_reading);

/// A collection of style information for card and set fields
class StyleSheet : public Packaged {
  public:
	StyleSheet();
	
	GameP game;						///< The game this stylesheet is made for
	OptionalScript init_script;		///< Script of variables available to other scripts in this stylesheet
	double card_width;				///< The width of a card in pixels
	double card_height;				///< The height of a card in pixels
	double card_dpi;				///< The resolution of a card in dots per inch
	Color  card_background;			///< The background color of cards
	/// The styling for card fields
	/** The indices should correspond to the card_fields in the Game */
	IndexMap<FieldP, StyleP> card_style;
	/// The styling for set info fields
	/** The indices should correspond to the set_fields in the Game */
	IndexMap<FieldP, StyleP> set_info_style;
	/// Extra card fields for boxes and borders
	vector<FieldP> extra_card_fields;
	IndexMap<FieldP, StyleP> extra_card_style;
	/// Extra fields for styling options
	vector<FieldP> styling_fields;
	/// The styling for the extra set fields
	/** The indices should correspond to the styling_fields */
	IndexMap<FieldP, StyleP> styling_style;
	
	bool dependencies_initialized;	///< are the script dependencies comming from this stylesheet all initialized?
	
	inline RealRect getCardRect() const { return RealRect(0, 0, card_width, card_height); }
	
	/// Return the style for a given field, it is not specified what type of field this is.
	StyleP styleFor(const FieldP& field);
	
	/// Load a StyleSheet, given a Game and the name of the StyleSheet
	static StyleSheetP byGameAndName(const Game& game, const String& name);
	/// name of the package without the game name
	String stylesheetName() const;
	
	static String typeNameStatic();
	virtual String typeName() const;
	Version fileVersion() const;
	/// Validate the stylesheet
	virtual void validate(Version = app_version);
	
  protected:
	
	DECLARE_REFLECTION();
};

inline String type_name(const StyleSheet&) {
	return _TYPE_("stylesheet");
}

void mark_dependency_value(const StyleSheet& value, const Dependency& dep);

// ----------------------------------------------------------------------------- : EOF
#endif
