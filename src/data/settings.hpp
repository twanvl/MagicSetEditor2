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

class Game;
class CardStyle;

DECLARE_POINTER_TYPE(GameSettings);
DECLARE_POINTER_TYPE(StyleSettings);

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
	UInt width;
	int  position;
	bool visible;
	
	DECLARE_REFLECTION();
};

/// Settings for a Game
class GameSettings {
  public:
	String                      default_style;
	String                      default_export;
	map<String, ColumnSettings> columns;
	String                      sort_cards_by;
	bool                        sort_cards_ascending;
	
	DECLARE_REFLECTION();
};

/// Settings for a Style
class StyleSettings {
  public:
	// Rendering/display settings
/*	SimpleDefaultable<double> card_zoom          = 1.0;
	SimpleDefaultable<int>    card_angle         = 0;
	SimpleDefaultable<bool>   card_anti_alias    = true;
	SimpleDefaultable<bool>   card_borders       = true;
	SimpleDefaultable<bool>   card_normal_export = true;
*/	
	DECLARE_REFLECTION();
	
//	/// Where the settings are the default, use the value from ss
//	void useDefault(const StyleSettings& ss);
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
	
	// --------------------------------------------------- : Game/style specific
	
	/// Get the settings object for a specific game
	GameSettings& gameSettingsFor(const Game& game);
	/// Get the settings object for a specific style
	StyleSettings& styleSettingsFor(const CardStyle& style);
	
  private:
	map<String,GameSettingsP> game_settings;
	map<String,StyleSettingsP> style_settings;
	StyleSettings default_style_settings;
  public:
	
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
