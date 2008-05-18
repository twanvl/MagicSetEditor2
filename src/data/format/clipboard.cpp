//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/clipboard.hpp>
#include <data/format/formats.hpp>
#include <data/card.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/keyword.hpp>
#include <util/io/package.hpp>
#include <script/scriptable.hpp>
#include <wx/sstream.h>

// ----------------------------------------------------------------------------- : Clipboard serialization

template <typename T>
String serialize_for_clipboard(Package& package, T& object) {
	shared_ptr<wxStringOutputStream> stream( new wxStringOutputStream );
	Writer writer(stream);
	WITH_DYNAMIC_ARG(clipboard_package, &package);
		writer.handle(object);
	return stream->GetString();
}

template <typename T>
void deserialize_from_clipboard(T& object, Package& package, const String& data) {
	shared_ptr<wxStringInputStream> stream( new wxStringInputStream(data) );
	Reader reader(stream, nullptr, _("clipboard"));
	WITH_DYNAMIC_ARG(clipboard_package, &package);
		reader.handle_greedy(object);
}

// ----------------------------------------------------------------------------- : CardDataObject

/// A wrapped cards for storing on the clipboard
struct WrappedCards {
	Game*         expected_game;
	String        game_name;
	vector<CardP> cards;
	
	DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(WrappedCards) {
	REFLECT(game_name);
	if (game_name == expected_game->name()) {
		WITH_DYNAMIC_ARG(game_for_reading, expected_game);
		REFLECT(cards);
	}
}


wxDataFormat CardsDataObject::format = _("application/x-mse-cards");

CardsDataObject::CardsDataObject(const SetP& set, const vector<CardP>& cards) {
	// set the stylesheet, so when deserializing we know whos style options we are reading
	bool* has_styling = new bool[cards.size()];
	for (size_t i = 0 ; i < cards.size() ; ++i) {
		has_styling[i] = cards[i]->has_styling && !cards[i]->stylesheet;
		if (has_styling[i]) {
			cards[i]->stylesheet = set->stylesheet;
		}
	}
	WrappedCards data = { set->game.get(), set->game->name(), cards };
	SetText(serialize_for_clipboard(*set, data));
	// restore cards
	for (size_t i = 0 ; i < cards.size() ; ++i) {
		if (has_styling[i]) {
			cards[i]->stylesheet = StyleSheetP();
		}
	}
	SetFormat(format);
	delete [] has_styling;
}

CardsDataObject::CardsDataObject() {
	SetFormat(format);
}

bool CardsDataObject::getCards(const SetP& set, vector<CardP>& out) {
	WrappedCards data = { set->game.get(), set->game->name() };
	deserialize_from_clipboard(data, *set, GetText());
	if (data.cards.empty()) return false;
	if (data.game_name == set->game->name()) {
		// Cards are from the same game
		out = data.cards;
		return true;
	} else {
		return false;
	}
}

// ----------------------------------------------------------------------------- : KeywordDataObject

/// A wrapped keyword for storing on the clipboard
struct WrappedKeyword {
	Game*    expected_game;
	String   game_name;
	KeywordP keyword;
	
	DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(WrappedKeyword) {
	REFLECT(game_name);
	if (game_name == expected_game->name()) {
		WITH_DYNAMIC_ARG(game_for_reading, expected_game);
		REFLECT(keyword);
	}
}


wxDataFormat KeywordDataObject::format = _("application/x-mse-keyword");

KeywordDataObject::KeywordDataObject(const SetP& set, const KeywordP& keyword) {
	WrappedKeyword data = { set->game.get(), set->game->name(), keyword };
	SetText(serialize_for_clipboard(*set, data));
	SetFormat(format);
}

KeywordDataObject::KeywordDataObject() {
	SetFormat(format);
}

KeywordP KeywordDataObject::getKeyword(const SetP& set) {
	KeywordP keyword(new Keyword());
	WrappedKeyword data = { set->game.get(), set->game->name(), keyword};
	deserialize_from_clipboard(data, *set, GetText());
	if (data.game_name != set->game->name()) return KeywordP(); // Keyword is from a different game
	else                                     return keyword;
}

// ----------------------------------------------------------------------------- : Card on clipboard

CardsOnClipboard::CardsOnClipboard(const SetP& set, const vector<CardP>& cards) {
	// Conversion to text format
		// TODO
		//Add( new TextDataObject(_("card"))) 
	// Conversion to bitmap format
		if (cards.size() == 1) {
			Add(new wxBitmapDataObject(export_bitmap(set, cards[0])));
		}
	// Conversion to serialized card format
		Add(new CardsDataObject(set, cards), true);
}
