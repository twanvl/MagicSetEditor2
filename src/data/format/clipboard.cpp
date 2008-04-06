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

/// A wrapped card for storing on the clipboard
struct WrappedCard {
	Game*  expected_game;
	String game_name;
	CardP  card;
	
	DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(WrappedCard) {
	REFLECT(game_name);
	if (game_name == expected_game->name()) {
		WITH_DYNAMIC_ARG(game_for_reading, expected_game);
		REFLECT(card);
	}
}


wxDataFormat CardDataObject::format = _("application/x-mse-card");

CardDataObject::CardDataObject(const SetP& set, const CardP& card) {
	WrappedCard data = { set->game.get(), set->game->name(), card };
	bool has_styling = card->has_styling && !card->stylesheet;
	if (has_styling) {
		// set the stylsheet, so when deserializing we know whos style options we are reading
		card->stylesheet = set->stylesheet;
	}
	SetText(serialize_for_clipboard(*set, data));
	if (has_styling) {
		card->stylesheet = StyleSheetP(); // restore card
	}
	SetFormat(format);
}

CardDataObject::CardDataObject() {
	SetFormat(format);
}

CardP CardDataObject::getCard(const SetP& set) {
	CardP card(new Card(*set->game));
	WrappedCard data = { set->game.get(), set->game->name(), card};
	deserialize_from_clipboard(data, *set, GetText());
	if (data.game_name != set->game->name()) return CardP(); // Card is from a different game
	else                                     return card;
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

CardOnClipboard::CardOnClipboard(const SetP& set, const CardP& card) {
	// Conversion to text format
		// TODO
		//Add( new TextDataObject(_("card"))) 
	// Conversion to bitmap format
		Add(new wxBitmapDataObject(export_bitmap(set, card)));
	// Conversion to serialized card format
		Add(new CardDataObject(set, card), true);
}
