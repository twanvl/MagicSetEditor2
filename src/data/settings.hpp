//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_SETTINGS
#define HEADER_DATA_SETTINGS

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/defaultable.hpp>

class Game;
class StyleSheet;
class Field;

DECLARE_POINTER_TYPE(GameSettings);
DECLARE_POINTER_TYPE(StyleSheetSettings);

// ----------------------------------------------------------------------------- : Extra data structures

/// When to check for updates?
enum CheckUpdates
{	CHECK_ALWAYS
,	CHECK_IF_CONNECTED
,	CHECK_NEVER
};

/// Settings of a single column in the card list
class ColumnSettings {
  public:
	ColumnSettings();
	UInt width;
	int  position;
	bool visible;
	
	DECLARE_REFLECTION();
};

/// Settings for a Game
class GameSettings {
  public:
	GameSettings();
	
	String                      default_stylesheet;
	String                      default_export;
	map<String, ColumnSettings> columns;
	String                      sort_cards_by;
	bool                        sort_cards_ascending;
	
	DECLARE_REFLECTION();
};

/// Settings for a StyleSheet
class StyleSheetSettings {
  public:
	StyleSheetSettings();
	
	// Rendering/display settings
	Defaultable<double> card_zoom;
	Defaultable<int>    card_angle;
	Defaultable<bool>   card_anti_alias;
	Defaultable<bool>   card_borders;
	Defaultable<bool>   card_normal_export;
	
	/// Where the settings are the default, use the value from ss
	void useDefault(const StyleSheetSettings& ss);
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Settings

/// Class that holds MSE settings.
/** There is a single global instance of this class.
 *  Settings are loaded at startup, and stored at shutdown.
 */
class Settings {
  public:
	/// Default constructor initializes default settings
	Settings();
	
	// --------------------------------------------------- : Locale
	
	String locale;
	
	// --------------------------------------------------- : Recently opened sets
	vector<String> recent_sets;
	static const UInt max_recent_sets = 4; // store this many recent sets
	
	/// Add a file to the list of recent files
	void addRecentFile(const String& filename);
	
	// --------------------------------------------------- : Set window size
	bool set_window_maximized;
	UInt set_window_width;
	UInt set_window_height;
	UInt card_notes_height;
	
	// --------------------------------------------------- : Default pacakge selections
	String default_game;
	
	// --------------------------------------------------- : Game/stylesheet specific
	
	/// Get the settings object for a specific game
	GameSettings&       gameSettingsFor      (const Game& game);
	/// Get the settings for a column for a specific field in a game
	ColumnSettings&     columnSettingsFor    (const Game& game, const Field& field);
	/// Get the settings object for a specific stylesheet
	StyleSheetSettings& stylesheetSettingsFor(const StyleSheet& stylesheet);
	
  private:
	map<String,GameSettingsP>       game_settings;
	map<String,StyleSheetSettingsP> stylesheet_settings;
  public:
	StyleSheetSettings              default_stylesheet_settings;	///< The default settings for stylesheets
	
	// --------------------------------------------------- : Special game stuff
	String apprentice_location;
	String mws_location;
	
	// --------------------------------------------------- : Update checking
	String updates_url;
	CheckUpdates check_updates;
	
	// --------------------------------------------------- : The io
	
	/// Read the settings file from the standard location
	void read();
	/// Store the settings in the standard location
	void write();
	
  private:
	/// Name of the settings file
	String settingsFile();
	
	DECLARE_REFLECTION();
};

/// The global settings object
extern Settings settings;

// ----------------------------------------------------------------------------- : EOF
#endif
