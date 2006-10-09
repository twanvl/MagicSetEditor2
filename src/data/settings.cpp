//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/settings.hpp>
#include <data/game.hpp>
#include <util/reflect.hpp>
#include <util/io/reader.hpp>
#include <util/io/writer.hpp>
#include <wx/filename.h>
#include <wx/wfstream.h>

// ----------------------------------------------------------------------------- : Extra types

IMPLEMENT_REFLECTION_ENUM(CheckUpdates) {
	VALUE_N("if connected", CHECK_IF_CONNECTED); //default
	VALUE_N("always",       CHECK_ALWAYS);
	VALUE_N("never",        CHECK_NEVER);
}

IMPLEMENT_REFLECTION(ColumnSettings) {
	REFLECT(width);
	REFLECT(position);
	REFLECT(visible);
}

IMPLEMENT_REFLECTION(GameSettings) {
	REFLECT_N("default style",        defaultStyle);
	REFLECT_N("default export",       defaultExport);
//	REFLECT_N("cardlist columns",     columns);
	REFLECT_N("sort cards by",        sortCardsBy);
	REFLECT_N("sort cards ascending", sortCardsAscending);
}

IMPLEMENT_REFLECTION(StyleSettings) {
	// TODO
}

// ----------------------------------------------------------------------------- : Settings

Settings settings;

Settings::Settings()
	: setWindowMaximized (false)
	, setWindowWidth     (790)
	, setWindowHeight    (300)
	, cardNotesHeight     (40)
	, updatesUrl          (_("http://magicseteditor.sourceforge.net/updates"))
	, checkUpdates        (CHECK_IF_CONNECTED)
{}

void Settings::addRecentFile(const String& filename) {
	// get absolute path
	wxFileName fn(filename);
	fn.Normalize();
	String filenameAbs = fn.GetFullPath();
	// remove duplicates
	recentSets.erase(
		remove(recentSets.begin(), recentSets.end(), filenameAbs),
		recentSets.end()
	);
	// add to front of list
	recentSets.insert(recentSets.begin(), filenameAbs);
	// enforce size limit
	if (recentSets.size() > maxRecentSets) recentSets.resize(maxRecentSets);
}

GameSettings& Settings::gameSettingsFor(const Game& game) {
	GameSettingsP& gs = settings.gameSettings[game.name()];
	if (!gs) gs.reset(new GameSettings);
	return *gs;
}
/*
StyleSettings& Settings::styleSettingsFor(const CardStyle& style) {
	StyleSettingsP& ss = settings.styleSettings#(style.name());
	if (!ss)  ss = new_shared<StyleSettings>();
	ss->useDefault(defaultStyleSettings); // update default settings
	return *ss;
}
*/

String userSettingsDir() {
	return _(""); // TODO
}

String Settings::settingsFile() {
//	return userSettingsDir() + _("mse.config");
	return userSettingsDir() + _("mse8.config"); // use different file during development of C++ port
}

IMPLEMENT_REFLECTION(Settings) {
//	ioMseVersion(io, "settings", fileVersion);
	REFLECT_N("recent set",          recentSets);
	REFLECT_N("window maximized",    setWindowMaximized);
	REFLECT_N("window width",        setWindowWidth);
	REFLECT_N("window height",       setWindowHeight);
	REFLECT_N("card notes height",   cardNotesHeight);
	REFLECT_N("default game",        defaultGame);
	REFLECT_N("apprentice location", apprenticeLocation);
	REFLECT_N("updates url",         updatesUrl);
	REFLECT_N("check updates",       checkUpdates);
//	ioAll(io, "game settings", gameSettings);
//	ioStyleSettings(io);
	REFLECT_N("default style settings", defaultStyleSettings);
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
