//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/clipboard.hpp>
#include <data/format/formats.hpp>
#include <data/card.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
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
	Reader reader(stream, _("clipboard"));
	WITH_DYNAMIC_ARG(clipboard_package, &package);
		reader.handle_greedy(object);
}

// ----------------------------------------------------------------------------- : CardDataObject

/// A wrapped card for storing on the clipboard
struct WrappedCard {
	String expected_game_name;
	String game_name;
	CardP  card;
	
	DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(WrappedCard) {
	if (game_name == expected_game_name) REFLECT(game_name);
	if (game_name == expected_game_name) REFLECT(card);
}


wxDataFormat CardDataObject::format = _("application/x-mse-card");

CardDataObject::CardDataObject(const SetP& set, const CardP& card) {
	WrappedCard data = { set->game->name(), set->game->name(), card };
	SetText(serialize_for_clipboard(*set, data));
	SetFormat(format);
}

CardDataObject::CardDataObject() {
	SetFormat(format);
}

CardP CardDataObject::getCard(const SetP& set) {
	CardP card(new Card(*set->game));
	WrappedCard data = { set->game->name(), set->game->name(), card};
	deserialize_from_clipboard(data, *set, GetText());
	if (data.game_name != set->game->name()) return CardP(); // Card is from a different game
	else                                     return card;
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
