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

String StyleSheet::fullName() const { return full_name; }
InputStreamP StyleSheet::openIconFile() {
	if (!icon_filename.empty()) {
		return openIn(icon_filename);
	} else {
		return game->openIconFile(); // use game icon by default
	}
}

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
	tag.addAlias(300, _("width"),      _("card width"));
	tag.addAlias(300, _("height"),     _("card height"));
	tag.addAlias(300, _("dpi"),        _("card dpi"));
	tag.addAlias(300, _("background"), _("card background"));
	tag.addAlias(300, _("info style"), _("set info style"));
	tag.addAlias(300, _("align"),      _("alignment"));
	tag.addAlias(300, _("extra field"),_("styling field"));
	tag.addAlias(300, _("extra style"),_("styling style"));
	
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
	REFLECT(styling_fields);
	if (tag.reading()) styling_style.init(styling_fields);
	REFLECT(styling_style);
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
	if (stylesheet) handle(stylesheet->stylesheetName());
}
