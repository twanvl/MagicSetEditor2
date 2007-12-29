//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/stylesheet.hpp>
#include <data/game.hpp>
#include <data/field.hpp>
#include <util/io/package_manager.hpp>
#include <gui/new_window.hpp> // for selecting stylesheets on load error

DECLARE_TYPEOF_COLLECTION(StyleSheet*);
DECLARE_TYPEOF_COLLECTION(FieldP);

// ----------------------------------------------------------------------------- : StyleSheet

IMPLEMENT_DYNAMIC_ARG(StyleSheet*, stylesheet_for_reading, nullptr);

StyleSheet::StyleSheet()
	: card_width(100), card_height(100)
	, card_dpi(96), card_background(*wxWHITE)
	, dependencies_initialized(false)
{}

StyleSheetP StyleSheet::byGameAndName(const Game& game, const String& name) {
	/// Alternative stylesheets for game
	static map<String, String> stylesheet_alternatives;
	String full_name = game.name() + _("-") + name + _(".mse-style");
	try {
		map<String, String>::const_iterator it = stylesheet_alternatives.find(full_name);
		if (it != stylesheet_alternatives.end()) {
			return packages.open<StyleSheet>(it->second);
		} else {
			return packages.open<StyleSheet>(full_name);
		}
	} catch (PackageNotFoundError& e) {
		if (stylesheet_for_reading()) {
			// we already have a stylesheet higher up, so just return a null pointer
			return StyleSheetP();
		}
		// load an alternative stylesheet
		StyleSheetP ss = select_stylesheet(game, name);
		if (ss) {
			stylesheet_alternatives[full_name] = ss->relativeFilename();
			return ss;
		} else {
			throw e;
		}
	}
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

void StyleSheet::validate(Version ver) {
	Packaged::validate(ver);
	if (!game) {
		throw Error(_ERROR_("no game specified for stylesheet"));
	}
	// a stylsheet depends on the game it is made for
	requireDependency(game.get());
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
	REFLECT(card_width);
	REFLECT(card_height);
	REFLECT(card_dpi);
	REFLECT(card_background);
	REFLECT(init_script);
	// styling
	REFLECT(styling_fields);
	REFLECT_IF_READING styling_style.init(styling_fields);
	REFLECT(styling_style);
	// style of game fields
	if (game) {
		REFLECT_IF_READING {
			card_style.init(game->card_fields);
			set_info_style.cloneFrom(game->default_set_style);
		}
		REFLECT(set_info_style);
		REFLECT(card_style);
	}
	// extra card fields
	REFLECT(extra_card_fields);
	REFLECT_IF_READING {
		if (extra_card_style.init(extra_card_fields)) {
			// if a value is not editable, don't save it
			FOR_EACH(f, extra_card_fields) {
				if (!f->editable) f->save_value = false;
			}
		}
	}
	REFLECT(extra_card_style);
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
