//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/game.hpp>
#include <data/field.hpp>
#include <util/io/package_manager.hpp>
#include <script/value.hpp>

// ----------------------------------------------------------------------------- : Game

GameP Game::byName(const String& name) {
	return packages.open<Game>(name + _(".mse-game"));
}

bool Game::isMagic() const {
	return name() == _("magic");
}

String Game::typeName() const { return _("game"); }

IMPLEMENT_REFLECTION(Game) {
//	ioMseVersion(io, fileName, fileVersion);
	REFLECT_N("full name",     fullName);
	REFLECT_N("icon",          iconFilename);
//	REFLECT_N("init script",   initScript);
	REFLECT_N("set field",     setFields);
	REFLECT_N("card field",    cardFields);
//	REFLECT_N("keyword parameter type", keywordParams);
//	REFLECT_N("keyword separator type", keywordSeparators);
//	REFLECT_N("keyword",      keywords);
//	REFLECT_N("word list",    wordLists);
}

void Game::validate() {
	// a default for the full name
	if (fullName.empty()) fullName = name();
}