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
	REFLECT(full_name);
	REFLECT_N("icon",          icon_filename);
//	REFLECT(init_script);
	REFLECT(set_fields);
	REFLECT(card_fields);
//	REFLECT_N("keyword parameter type", keyword_params);
//	REFLECT_N("keyword separator type", keyword_separators);
//	REFLECT_N("keyword",      keywords);
//	REFLECT_N("word list",    word_lists);
}

void Game::validate() {
	// a default for the full name
	if (full_name.empty()) full_name = name();
}