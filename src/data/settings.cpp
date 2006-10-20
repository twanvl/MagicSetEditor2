//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/settings.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <util/reflect.hpp>
#include <util/io/reader.hpp>
#include <util/io/writer.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <script/value.hpp>

// ----------------------------------------------------------------------------- : Extra types

IMPLEMENT_REFLECTION_ENUM(CheckUpdates) {
	VALUE_N("if connected", CHECK_IF_CONNECTED); //default
	VALUE_N("always",       CHECK_ALWAYS);
	VALUE_N("never",        CHECK_NEVER);
}

const int COLUMN_NOT_INITIALIZED = -100000;

ColumnSettings::ColumnSettings()
	: width(100), position(COLUMN_NOT_INITIALIZED), visible(false)
{}

IMPLEMENT_REFLECTION(ColumnSettings) {
	REFLECT(width);
	REFLECT(position);
	REFLECT(visible);
}

IMPLEMENT_REFLECTION(GameSettings) {
	REFLECT(default_stylesheet);
	REFLECT(default_export);
//	REFLECT_N("cardlist columns",     columns);
	REFLECT(sort_cards_by);
	REFLECT(sort_cards_ascending);
}

IMPLEMENT_REFLECTION(StyleSheetSettings) {
	// TODO
}

// ----------------------------------------------------------------------------- : Settings

Settings settings;

Settings::Settings()
	: set_window_maximized (false)
	, set_window_width     (790)
	, set_window_height    (300)
	, card_notes_height    (40)
	, updates_url          (_("http://magicseteditor.sourceforge.net/updates"))
	, check_updates        (CHECK_IF_CONNECTED)
{}

void Settings::addRecentFile(const String& filename) {
	// get absolute path
	wxFileName fn(filename);
	fn.Normalize();
	String filenameAbs = fn.GetFullPath();
	// remove duplicates
	recent_sets.erase(
		remove(recent_sets.begin(), recent_sets.end(), filenameAbs),
		recent_sets.end()
	);
	// add to front of list
	recent_sets.insert(recent_sets.begin(), filenameAbs);
	// enforce size limit
	if (recent_sets.size() > max_recent_sets) recent_sets.resize(max_recent_sets);
}

GameSettings& Settings::gameSettingsFor(const Game& game) {
	GameSettingsP& gs = settings.game_settings[game.name()];
	if (!gs) gs.reset(new GameSettings);
	return *gs;
}
ColumnSettings& Settings::columnSettingsFor(const Game& game, const Field& field) {
	// Get game info
	GameSettings& gs = gameSettingsFor(game);
	// Get column info
	ColumnSettings& cs = gs.columns[field.name];
	if (cs.position == COLUMN_NOT_INITIALIZED) {
		// column info not set, initialize based on the game
		cs.visible  = field.card_list_column >= 0;
		cs.position = field.card_list_column;
		cs.width    = field.card_list_width;
	}
	return cs;
}
/*
StyleSettings& Settings::styleSettingsFor(const CardStyle& style) {
	StyleSettingsP& ss = settings.styleSettings#(style.name());
	if (!ss)  ss = new_shared<StyleSettings>();
	ss->useDefault(defaultStyleSettings); // update default settings
	return *ss;
}
*/

String user_settings_dir() {
	return _(""); // TODO
}

String Settings::settingsFile() {
//	return user_settings_dir() + _("mse.config");
	return user_settings_dir() + _("mse8.config"); // use different file during development of C++ port
}

IMPLEMENT_REFLECTION(Settings) {
//	ioMseVersion(io, "settings", file_version);
	REFLECT(recent_sets);
	REFLECT(set_window_maximized);
	REFLECT(set_window_width);
	REFLECT(set_window_height);
	REFLECT(card_notes_height);
	REFLECT(default_game);
	REFLECT(apprentice_location);
	REFLECT(updates_url);
	REFLECT(check_updates);
//	ioAll(io, game_settings);
//	ioStyleSettings(io);
//	REFLECT(default_style_settings);
}

void Settings::read() {
	String filename = settingsFile();
	if (wxFileExists(filename)) {
		// settings file not existing is not an error
		shared_ptr<wxFileInputStream> file = new_shared1<wxFileInputStream>(filename);
		if (!file->Ok()) return; // failure is not an error
		Reader reader(file, filename);
		reader.handle(*this);
	}
}

void Settings::write() {
	Writer writer(new_shared1<wxFileOutputStream>(settingsFile()));
	writer.handle(*this);
}
