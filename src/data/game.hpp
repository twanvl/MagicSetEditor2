//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_GAME
#define HEADER_DATA_GAME

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <script/scriptable.hpp>
#include <script/dependency.hpp>
#include <util/dynamic_arg.hpp>

DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StatsDimension);
DECLARE_POINTER_TYPE(StatsCategory);
DECLARE_POINTER_TYPE(PackType);
DECLARE_POINTER_TYPE(KeywordParam);
DECLARE_POINTER_TYPE(KeywordMode);
DECLARE_POINTER_TYPE(Keyword);
DECLARE_POINTER_TYPE(WordList);
DECLARE_POINTER_TYPE(AddCardsScript);
DECLARE_POINTER_TYPE(AutoReplace);

// ----------------------------------------------------------------------------- : Game

/// Game that is used for cards constructed with the default constructor, as well as for reading stylesheets
DECLARE_DYNAMIC_ARG(Game*, game_for_reading);

/// A description of a card game
class Game : public Packaged {
  public:
	Game();
	
	OptionalScript          init_script;            ///< Script of variables available to other scripts in this game
	vector<FieldP>          set_fields;             ///< Fields for set information
	IndexMap<FieldP,StyleP> default_set_style;      ///< Default style for the set fields, because it is often the same
	vector<FieldP>          card_fields;            ///< Fields on each card
	OptionalScript          card_list_color_script;	///< Script that determines the color of items in the card list
	vector<StatsDimensionP> statistics_dimensions;  ///< (Additional) statistics dimensions
	vector<StatsCategoryP>  statistics_categories;  ///< (Additional) statistics categories
	vector<PackTypeP>       pack_types;				///< Types of random card packs to generate
	vector<WordListP>       word_lists;				///< Word lists for editing with a drop down list
	vector<AddCardsScriptP> add_cards_scripts;		///< Scripts for adding multiple cards to the set
	vector<AutoReplaceP>	auto_replaces;			///< Things to autoreplace in textboxes
	
	bool                    has_keywords;           ///< Does this game use keywords?
	OptionalScript          keyword_match_script;	///< For the keyword editor
	vector<KeywordParamP>   keyword_parameter_types;///< Types of keyword parameters
	vector<KeywordModeP>    keyword_modes;          ///< Modes of keywords
	vector<KeywordP>        keywords;               ///< Keywords for use in text
	
	Dependencies dependent_scripts_cards;           ///< scripts that depend on the card list
	Dependencies dependent_scripts_keywords;        ///< scripts that depend on the keywords
	Dependencies dependent_scripts_stylesheet;		///< scripts that depend on the card's stylesheet
	bool dependencies_initialized;                  ///< are the script dependencies comming from this game all initialized?
	
	/// Loads the game with a particular name, for example "magic"
	static GameP byName(const String& name);
	
	/// Is this Magic the Gathering?
	bool isMagic() const;
	
	/// Initialize card_list_color_script
	void initCardListColorScript();
	
	static String typeNameStatic();
	virtual String typeName() const;
	Version fileVersion() const;
	
  protected:
	virtual void validate(Version);
	
	DECLARE_REFLECTION();
};

inline String type_name(const Game&) {
	return _TYPE_("game");
}

// ----------------------------------------------------------------------------- : EOF
#endif
