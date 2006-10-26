//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_STYLESHEET
#define HEADER_DATA_STYLESHEET

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);

// ----------------------------------------------------------------------------- : StyleSheet

/// A collection of style information for card and set fields
class StyleSheet : public Packaged {
  public:
	StyleSheet();
	
	GameP game;						///< The game this stylesheet is made for
	String full_name;				///< Name of this game, for menus etc.
	String icon_filename;			///< Filename of icon to use in NewWindow
	OptionalScript init_script;		///< Script of variables available to other scripts in this stylesheet
	double card_width;				///< The width of a card in pixels
	double card_height;				///< The height of a card in pixels
	double card_dpi;				///< The resolution of a card in dots per inch
	Color  card_background;			///< The background color of cards
	/// The styling for card fields
	/** The indices should correspond to the set_fields in the Game */
	IndexMap<FieldP, StyleP> card_style;
	/// The styling for set info fields
	/** The indices should correspond to the set_fields in the Game */
	IndexMap<FieldP, StyleP> set_info_style;
	
	static String typeNameStatic();
	virtual String typeName() const;
	virtual String fullName() const;
	virtual InputStreamP openIconFile();
	
	/// Load a StyleSheet, given a Game and the name of the StyleSheet
	static StyleSheetP byGameAndName(const Game& game, const String& name);
	
	/// name of the package without the game name
	String styleName();
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
