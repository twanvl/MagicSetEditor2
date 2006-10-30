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

String StyleSheet::typeNameStatic() { return _("style"); }
String StyleSheet::typeName() const { return _("style"); }

String StyleSheet::fullName() const { return full_name; }
InputStreamP StyleSheet::openIconFile() {
	if (!icon_filename.empty()) {
		return openIn(icon_filename);
	} else {
		return game->openIconFile(); // use game icon by default
	}
}
IMPLEMENT_REFLECTION(StyleSheet) {
	// < 0.3.0 didn't use card_ prefix
	tag.addAlias(300, _("width"),      _("card_width"));
	tag.addAlias(300, _("height"),     _("card_height"));
	tag.addAlias(300, _("dpi"),        _("card_dpi"));
	tag.addAlias(300, _("background"), _("card_background"));
	tag.addAlias(300, _("info_style"), _("set_info_style"));
	tag.addAlias(300, _("align"),      _("alignment"));
	
	REFLECT(game);
	REFLECT(full_name);
	REFLECT_N("icon",          icon_filename);
	REFLECT(init_script);
	REFLECT(card_width);
	REFLECT(card_height);
	REFLECT(card_dpi);
	REFLECT(card_background);
	if (game) {
		if (tag.reading()) {
			card_style    .init(game->card_fields);
			set_info_style.init(game->set_fields);
		}
		REFLECT(card_style);
		REFLECT(set_info_style);
	}
//	io(_("extra field"), extraSetFields);
//	extraInfoStyle.init(extraSetFields);
//	io(_("extra style"), extraInfoStyle);
}

void StyleSheet::validate(Version) {
	// a default for the full name
	if (full_name.empty()) full_name = name();
}


// special behaviour of reading/writing StyleSheetPs: only read/write the name

void Reader::handle(StyleSheetP& stylesheet) {
	if (!game_for_reading()) {
		throw InternalError(_("game_for_reading not set"));
	}
	stylesheet = StyleSheet::byGameAndName(*game_for_reading(), value);
}
void Writer::handle(const StyleSheetP& stylesheet) {
	handle(stylesheet->name());
}
