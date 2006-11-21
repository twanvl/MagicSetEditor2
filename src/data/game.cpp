//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/game.hpp>
#include <data/field.hpp>
#include <data/keyword.hpp>
#include <data/statistics.hpp>
#include <util/io/package_manager.hpp>
#include <script/script.hpp>
#include <script/value.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(StatsDimensionP);

// ----------------------------------------------------------------------------- : Game

IMPLEMENT_DYNAMIC_ARG(Game*, game_for_reading, nullptr);

Game::Game()
	: dependencies_initialized(false)
{}

GameP Game::byName(const String& name) {
	return packages.open<Game>(name + _(".mse-game"));
}

bool Game::isMagic() const {
	return name() == _("magic");
}

String Game::typeNameStatic() { return _("game"); }
String Game::typeName() const { return _("game"); }

String Game::fullName() const { return full_name; }
InputStreamP Game::openIconFile() {
	if (!icon_filename.empty()) {
		return openIn(icon_filename);
	} else {
		return InputStreamP();
	}
}

IMPLEMENT_REFLECTION(Game) {
	REFLECT(full_name);
	REFLECT_N("icon",          icon_filename);
	REFLECT(init_script);
	REFLECT(set_fields);
	REFLECT(card_fields);
	REFLECT(statistics_dimensions);
	REFLECT(statistics_categories);
	REFLECT(keyword_parameter_types);
	REFLECT(keyword_modes);
	REFLECT(keywords);
//	REFLECT(word_lists);
}

void Game::validate(Version) {
	// a default for the full name
	if (full_name.empty()) full_name = name();
	// automatic statistics dimensions
	{
		vector<StatsDimensionP> dims;
		FOR_EACH(f, card_fields) {
			if (f->show_statistics) {
				dims.push_back(new_shared1<StatsDimension>(*f));
			}
		}
		statistics_dimensions.insert(statistics_dimensions.begin(), dims.begin(), dims.end()); // push front
	}
	// automatic statistics categories
	{
		vector<StatsCategoryP> cats;
		FOR_EACH(dim, statistics_dimensions) {
			cats.push_back(new_shared1<StatsCategory>(dim));
		}
		statistics_categories.insert(statistics_categories.begin(), cats.begin(), cats.end()); // push front
	}
}

void addStatsDimensionsForFields();

// special behaviour of reading/writing GamePs: only read/write the name

void Reader::handle(GameP& game) {
	game = Game::byName(value);
}
void Writer::handle(const GameP& game) {
	if (game) handle(game->name());
}
