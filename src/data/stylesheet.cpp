//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/stylesheet.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <util/io/package_manager.hpp>

DECLARE_TYPEOF_COLLECTION(StyleSheet*);

// ----------------------------------------------------------------------------- : StyleSheet

StyleSheet::StyleSheet()
	: card_width(100), card_height(100)
	, card_dpi(96), card_background(*wxWHITE)
	, dependencies_initialized(false)
{}

StyleSheetP StyleSheet::byGameAndName(const Game& game, const String& name) {
	return packages.open<StyleSheet>(game.name() + _("-") + name + _(".mse-style"));
}
String StyleSheet::stylesheetName() const {
	String sn = name(), gn = game->name();
	if (sn.size() + 1 > gn.size()) {
		return sn.substr(gn.size() + 1); // remove "gamename-"
	} else {
		return sn;
	}
}

String StyleSheet::typeNameStatic() { return _("style"); }
String StyleSheet::typeName() const { return _("style"); }

StyleP StyleSheet::styleFor(const FieldP& field) {
	if (card_style.containsKey(field)) {
		return card_style[field];
	} else if (set_info_style.containsKey(field)) {
		return set_info_style[field];
	} else if (styling_style.containsKey(field)) {
		return styling_style[field];
	} else {
		throw InternalError(_("Can not find styling for field '")+field->name+_("'in stylesheet"));
	}
}


IMPLEMENT_REFLECTION(StyleSheet) {
	// < 0.3.0 didn't use card_ prefix
	REFLECT_ALIAS(300, "width",       "card width");
	REFLECT_ALIAS(300, "height",      "card height");
	REFLECT_ALIAS(300, "dpi",         "card dpi");
	REFLECT_ALIAS(300, "background",  "card background");
	REFLECT_ALIAS(300, "info style",  "set info style");
	REFLECT_ALIAS(300, "align",       "alignment");
	REFLECT_ALIAS(300, "extra field", "styling field");
	REFLECT_ALIAS(300, "extra style", "styling style");
	
	REFLECT(game);
	REFLECT_BASE(Packaged);
	REFLECT(init_script);
	REFLECT(card_width);
	REFLECT(card_height);
	REFLECT(card_dpi);
	REFLECT(card_background);
	if (game) {
		REFLECT_IF_READING {
			card_style    .init(game->card_fields);
			set_info_style.init(game->set_fields);
		}
		REFLECT(card_style);
		REFLECT(set_info_style);
	}
	REFLECT(styling_fields);
	REFLECT_IF_READING styling_style.init(styling_fields);
	REFLECT(styling_style);
}


// special behaviour of reading/writing StyleSheetPs: only read/write the name

void Reader::handle(StyleSheetP& stylesheet) {
	if (!game_for_reading()) {
		throw InternalError(_("game_for_reading not set"));
	}
	stylesheet = StyleSheet::byGameAndName(*game_for_reading(), getValue());
}
void Writer::handle(const StyleSheetP& stylesheet) {
	if (stylesheet) handle(stylesheet->stylesheetName());
}
